# time_tracker
Takes a file describing your day and generates another file with tables containing basic analysis.
An example can be found in `build/`.

#### Basic overview of the process:
(`time_raw.txt` + `time_cfg.txt`) => `time_tracker.exe` => `time_processed.txt`

Running `time_tracker.exe` will cause it to search for files with the same names as the ones above (`time_raw.txt` and `time_cfg.txt`) and will spit out `time_processed.txt.`

##### time_raw.txt
This is the file describing your days that gets fed into `time_tracker`. Some notes about it:
* The `@` and `#` at the beginning of each day are flags denoting whether the day should be processed (`@`) or not (`#`). If a day does get processed, `@` is changed to `#` by `time_tracker`. Everything else on the same line after these flags doesn't matter 
(so `@_________` is the same as `@`).
* Day of the week must remain in same place. Everything else on the same line after the day of the week doesn't matter 
(so `Monday | 02/10/2017 | (Day 15)` is the same as `Monday`).
* Newlines must conform to the example.
* `$ END` must be at the end of the file.

##### time_cfg.txt
This is the file describing time categories for each day that also gets fed into `time_tracker`. Some notes:
* Just about everything needs to be on its own line (i.e. the `{` and `}`s must be on their own line).
* `TargetTime` is the time elapsed you are shooting for for a particular category.
* `BadCondition` is the condition that describes whether or not the difference between `TargetTime` elapsed and `RealityTime` elapsed is good or bad.
* Time categories for days later in the file override previous time cateogories.
* `$ END` must be at the end of the file.

##### time_processed.txt
This is the file with the anaylsis tables. Some notes:
* `Target`shows the `TargetTime` from `time_cfg.txt`.
* `Reality` shows the actual time elapsed.
* `Result` shows the difference and whether or not it's over or under the `TargetTime`.
* The 'analysis' is on the right side.
* The `**Executing` entry for the last day is a dummy entry used for calculating the acutal last entry.
* The table with `****`s represents the last category entered.

### final notes
* Right now there is no way to change the number of rows, not because it would be hard, but because I prefer 3.
