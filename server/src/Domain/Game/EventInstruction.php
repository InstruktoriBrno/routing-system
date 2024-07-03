<?php
declare(strict_types=1);
namespace App\Domain\Game;

class EventInstruction
{
    public int $time;
    public GameRoundEventType $type;
    public string $parameter;
}
