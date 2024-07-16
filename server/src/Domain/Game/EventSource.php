<?php
declare(strict_types=1);
namespace App\Domain\Game;

enum EventSource: string
{
    case Online = 'online';
    case OfflineBox = 'offline_box';
    case OfflineCard = 'offline_card';
    case UI = 'ui';
}
