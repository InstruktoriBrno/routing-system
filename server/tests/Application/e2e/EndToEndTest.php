<?php
declare(strict_types=1);
namespace Tests\Application\e2e;

use Ivory\Connection\IConnection;
use Tests\TestCase;

abstract class EndToEndTest extends TestCase
{
    protected function setUp(): void
    {
        parent::setUp();
        $this->resetSampleData();
    }

    protected function tearDown(): void
    {
        $this->resetSampleData();
        parent::tearDown();
    }

    private function resetSampleData(): void
    {
        $sampleDataScript = file_get_contents(__DIR__ . '/../../../db-sample-data.sql');
        $db = $this->getDb();
        $db->runScript($sampleDataScript);
    }

    protected function getDb(): IConnection
    {
        return $this->app->getContainer()->get(IConnection::class);
    }
}
