<?php
declare(strict_types=1);
namespace App\Domain\Game;

use JsonException;
use stdClass;
use Swaggest\JsonSchema\Schema;
use Swaggest\JsonSchema\Exception as JsonSchemaException;

class GameRoundSpecValidator
{
    private const JSON_SCHEMA_PATH = __DIR__ . '/../../../../boxes/game_logic/data/round_schema.json';

    public function validate(stdClass $gameSpec): array
    {
        try {
            $schema = Schema::import(json_decode(file_get_contents(self::JSON_SCHEMA_PATH)));
        } catch (\Exception $e) {
            $path = (realpath(self::JSON_SCHEMA_PATH) ?: self::JSON_SCHEMA_PATH);
            throw new \RuntimeException("Error importing the JSON schema from `$path`", 0, $e);
        }

        try {
            $schema->in($gameSpec);
        } catch (JsonException $e) {
            return ['Not a valid JSON: ' . $e->getMessage()];
        } catch (JsonSchemaException $e) {
            return ['Invalid game round: JSON schema error: ' . $e->getMessage()];
        }

        return [];
    }
}
