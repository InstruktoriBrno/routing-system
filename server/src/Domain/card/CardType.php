<?php
declare(strict_types=1);
namespace App\Domain\Card;

enum CardType: string
{
    case Admin = 'admin';
    case Locator = 'locator';
    case TCP = 'tcp';
    case Chat = 'chat';
    case Priority = 'priority';
    case VisitAll = 'visitall';
}
