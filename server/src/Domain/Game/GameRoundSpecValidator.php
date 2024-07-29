<?php
declare(strict_types=1);
namespace App\Domain\Game;

use App\Domain\Card\CardType;
use JsonException;
use stdClass;
use Swaggest\JsonSchema\Schema;
use Swaggest\JsonSchema\Exception as JsonSchemaException;
use Swaggest\JsonSchema\SchemaContract;

class GameRoundSpecValidator
{
    private const JSON_SCHEMA_PATH = __DIR__ . '/../../../../boxes/game_logic/data/round_schema.json';

    private stdClass $gameSpec;
    private array $errors;

    public function validate(stdClass $gameSpec): array
    {
        $this->gameSpec = $gameSpec;
        $this->errors = [];

        $schema = $this->loadSchema();
        try {
            $schema->in($gameSpec);
        } catch (JsonException $e) {
            return ['Not a valid JSON: ' . $e->getMessage()];
        } catch (JsonSchemaException $e) {
            return ['Invalid game round: JSON schema error: ' . $e->getMessage()];
        }

        $this->checkRouters();
        $this->checkLinks();
        $this->checkPackets();
        $this->checkEvents();

        return $this->errors;
    }

    private function loadSchema(): SchemaContract
    {
        try {
            return Schema::import(json_decode(file_get_contents(self::JSON_SCHEMA_PATH)));
        } catch (\Exception $e) {
            $path = (realpath(self::JSON_SCHEMA_PATH) ?: self::JSON_SCHEMA_PATH);
            throw new \RuntimeException("Error importing the JSON schema from `$path`", 0, $e);
        }
    }

    private function checkRouters(): void
    {
        $macAddresses = [];
        foreach ($this->gameSpec->routers as $routerId => $routerDef) {
            $macList = $routerDef->mac;
            if (!$macList) {
                $this->routerError($routerId, 'empty MAC address list');
            }
            foreach ($macList as $mac) {
                if (in_array($mac, $macAddresses)) {
                    $this->routerError($routerId, "mapped to MAC address \"$mac\" which is mapped to another router");
                } else {
                    $macAddresses[] = $mac;
                }
            }
        }
    }

    private function checkLinks(): void
    {
        foreach ($this->gameSpec->links as $i => $link) {
            if (strlen($link) != 2) {
                $this->linkError($i, "invalid link definition: \"$link\"");
                continue;
            }
            
            foreach ([$link[0], $link[1]] as $routerId) {
                if (!$this->routerDefined($routerId)) {
                    $this->linkError($i, "router \"$routerId\" does not exist");
                }
            }
        }
    }

    private function checkPackets(): void
    {
        foreach ($this->gameSpec->packets as $cardNum => $spec) {
            if ($cardNum === '000' && $spec->type !== CardType::Admin->value) {
                $this->packetError($cardNum, sprintf('card number "%s" is reserved for type "%s"',
                    $cardNum,
                    CardType::Admin->value
                ));
            }

            if (isset($spec->source) && !$this->routerDefined($spec->source)) {
                $this->packetError($cardNum, "non-existent source router \"{$spec->source}\"");
            }
            if (isset($spec->destination) && !$this->routerDefined($spec->destination)) {
                $this->packetError($cardNum, "non-existent destination router \"{$spec->destination}\"");
            }
            if (isset($spec->source, $spec->destination) && $spec->source === $spec->destination) {
                $this->packetError($cardNum, "source same as destination");
            }
            if (isset($spec->releaseTime) && $spec->releaseTime >= $this->gameSpec->duration) {
                $this->packetError($cardNum, "releaseTime beyond the game duration of \"{$this->gameSpec->duration}\"");
            }
            if ($spec->type === CardType::Chat->value) {
                $rtc = $spec->roundTripCount;
                $msgCnt = count($spec->messages);
                if ($msgCnt != $rtc * 2) {
                    $this->packetError($cardNum, "wrong number of messages: expected 2*$rtc, got $msgCnt");
                }
            }
        }
    }

    private function checkEvents(): void
    {
        $linkActive = [];
        foreach ($this->gameSpec->links as $link) {
            $linkActive[self::normalizeLink($link)] = true;
        }

        $events = $this->gameSpec->events;
        uasort($events, function (stdClass $a, stdClass $b): int {
            return $a->time - $b->time;
        });
        foreach ($events as $eventIdx => $spec) {
            if ($spec->time >= $this->gameSpec->duration) {
                $this->eventError($eventIdx, "beyond the game duration of \"{$this->gameSpec->duration}\"");
            }

            switch ($spec->type) {
                case GameRoundEventType::LinkDown->value:
                    $link = self::normalizeLink($spec->link);
                    if (!isset($linkActive[$link])) {
                        $this->eventError($eventIdx, "non-existent link: \"{$spec->link}\"");
                    } elseif (!$linkActive[$link]) {
                        $this->eventError($eventIdx, "link already down: \"{$spec->link}\"");
                    } else {
                        $linkActive[$link] = false;
                    }
                    break;
                case GameRoundEventType::LinkUp->value:
                    $link = self::normalizeLink($spec->link);
                    if (!empty($linkActive[$link])) {
                        $this->eventError($eventIdx, "link already up: \"{$spec->link}\"");
                    } else {
                        $linkActive[$link] = true;
                    }
                    break;
                default:
                    $this->eventError($eventIdx, "unknown event type: \"{$spec->type}\"");
            }
        }
    }

    private function routerError(string $routerId, string $error): void
    {
        $this->errors[] = "routers[\"$routerId\"]: $error";
    }

    private function linkError(int $linkIdx, string $error): void
    {
        $this->errors[] = "links[$linkIdx]: $error";
    }

    private function packetError(string $cardNum, string $error): void
    {
        $this->errors[] = "packets[\"$cardNum\"]: $error";
    }

    private function eventError(int $eventIdx, string $error): void
    {
        $this->errors[] = "events[$eventIdx]: $error";
    }

    private function routerDefined(string $routerId): bool
    {
        return property_exists($this->gameSpec->routers, $routerId);
    }

    private static function normalizeLink(string $link): string
    {
        $endpoints = str_split($link);
        sort($endpoints);
        return implode('', $endpoints);
    }
}
