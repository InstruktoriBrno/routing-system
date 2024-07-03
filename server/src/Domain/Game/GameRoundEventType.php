<?php
declare(strict_types=1);
namespace App\Domain\Game;

enum GameRoundEventType: string
{
    case LinkUp = 'linkup';
    case LinkDown = 'linkdown';
}
