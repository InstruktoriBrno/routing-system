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
* The form will perform transformations from compact to full form of specs:
    * packet specs:
        * left-pad card numbers with zeroes to 3 digits ("1" -> "001")
        * transform packet definition from a list to object, e.g., ["standard", "A", "C", 10] transformed to { "type": "standard", "source": "A", "destination": "C" }
            * list[0]: packet type
            * list[1]: releaseTime
            * list[2]: "source" parameter, if relevant for the packet type
            * list[3..]: other packet type-specific parameters, in order as listed in the [Round specs](https://github.com/InstruktoriBrno/routing-system/blob/master/api.md#round-setup)

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
