<?php
declare(strict_types=1);
namespace App\Domain\Card;

enum CardType: string
{
    case Admin = 'admin';
    case Locator = 'locator';
    case Return = 'return';
    case Priority = 'priority';
    case VisitAll = 'visitall';
}
