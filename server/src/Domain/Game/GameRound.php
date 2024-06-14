<?php
declare(strict_types=1);
namespace App\Domain\Game;

class GameRound implements \JsonSerializable
{
    private ?int $id;
    private int $gameId;
    private string $name;
    private string $spec;
    private ?int $apiIdent;
    private ?string $apiPassword;

    public function __construct(?int $id, int $gameId, string $name, string $spec, ?int $apiIdent, ?string $apiPassword)
    {
        $this->id = $id;
        $this->gameId = $gameId;
        $this->name = $name;
        $this->spec = $spec;
        $this->apiIdent = $apiIdent;
        $this->apiPassword = $apiPassword;
    }

    public function getId(): ?int
    {
        return $this->id;
    }

    public function getGameId(): int
    {
        return $this->gameId;
    }

    public function getName(): string
    {
        return $this->name;
    }

    public function getSpec(): string
    {
        return $this->spec;
    }

    public function getApiIdent(): ?int
    {
        return $this->apiIdent;
    }

    public function getApiPassword(): string
    {
        return $this->apiPassword;
    }

    #[\ReturnTypeWillChange]
    public function jsonSerialize(): array
    {
        return [
            'id' => $this->id,
            'gameId' => $this->gameId,
            'name' => $this->name,
            'apiIdent' => $this->apiIdent,
        ];
    }
}
