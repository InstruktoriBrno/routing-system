<?php
declare(strict_types=1);
namespace App\Domain\Game;

class PacketInstruction
{
    public string $cardNum;
    public string $routerIdent;
    public ?int $releaseTime;
}
