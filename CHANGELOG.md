# LogAnalyzer Changelog

## 0.7
- you can now configure multiple log file sources in the settings
- files from local log file sources can now be added to the log file list
    - if files are added or removed from an active local folder the list with
      them will be reloaded
- now a log file can only exist once in the log file list
- improved removal speed of items from the log file list

## 0.6
- multiple log files can now be evaluated simultaneously by selecting multiple 
  items in the log file widget
    - the content of all selected log files will be open in the log text edit
    - ignore patterns can be used on all selected log files
    - reports will be generated over all selected log files
- ignore and report patterns can now be created by hitting <kbd>Return</kbd> 
  in the respective filter line edits
- start of settings dialog implementation to make log file sources configurable

## 0.5
- you can now generate a report from the report patterns
- you can export the report as PDF
- you can print the report
- the user interface of the pattern lists is slightly improved 

## 0.4
- report pattern CRUD, import and export
- adding of report patterns from selected text
    - use the shortcut <kbd>Ctrl</kbd> + <kbd>E</kbd>
- added a help menu entry to open the change log page
- added a help menu entry to open the issue page
- added a help menu entry to open the release page

## 0.3
- added a desktop icon

## 0.2
- added ignore pattern filtering
- added moving of ignore patterns via drag and drop

## 0.1
- adding log files via drag and drop
- finding text in the log file with <kbd>Ctrl</kbd> + <kbd>F</kbd>
    - navigate with <kbd>F3</kbd> and <kbd>⇧</kbd> + <kbd>F3</kbd>
- adding of ignore patterns from selected text
    - use the shortcut <kbd>Ctrl</kbd> + <kbd>I</kbd>
    - use regular expressions as patterns
- searching for matching text of ignore patterns if you select them
- removing all matches of ignore patterns from the text
    - empty lines will be removed by default
- exporting of ignore patterns with <kbd>Ctrl</kbd> + <kbd>⇧</kbd> + 
  <kbd>E</kbd>
- importing of ignore patterns with <kbd>Ctrl</kbd> + <kbd>⇧</kbd> + 
  <kbd>I</kbd>
- reloading of current file with <kbd>Ctrl</kbd> + <kbd>R</kbd>
