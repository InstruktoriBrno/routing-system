# User Interface Requirements

* HTTPS only.
* Protected with HTTP Basic Auth.

## Entry Screen
* List all games created in the system, sorted by date DESC, name ASC
    * Form to create new game
    * Link to game detail
* Link to round spec library

## Round Spec Library
* List all round specs
    * Link to edit a round spec -> prefills the form
    * Button to delete a round spec, with JS confirmation, sets the "deleted" flag
* Form to create new spec

## Game Detail
* Link to game setup
    * Game name, date
* Link to management of subteams
* Link to rounds management
* Link to game controls - selected game round
    * Upload round
    * Start game
    * Pause game
    * Resume game
    * Award ad hoc points
    * Network status, diagnostics
    * Router checkin status
    * Event log
* Link to scoreboards
    * Subteam scoreboard -> PDF (all subteams)
    * Overall scoreboard -> PDF
