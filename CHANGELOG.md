# LogAnalyzer Changelog

## 21.6.0
- fixed snap desktop icon (for [#9](https://github.com/pbek/loganalyzer/issues/9))
- added Ubuntu Launchpad builds for up to Ubuntu 21.10 (impish)
- added Debian builds for up to Debian 10 and Debian Unstable
- added openSUSE Leap builds for up to openSUSE Leap 15.3

## 18.11.3
- fixed windows build

## 18.11.2
- fixed icons

## 18.11.1
- you can now reorder the log files in the *Evaluation area*
- added a *Debug* tab in the settings to show and export information about your
  installation of LogAnalyzer
- there are now LogAnalyzer **snap packages** available for `amd64`, `i386`, 
  `arm64` and `armhf`
    - you can install it with `snap install loganalyzer-pbek` and run the 
      application with `loganalyzer-pbek`
- Qt versions for the Windows and macOS builds were updated to the current release

## 18.11.0
- added context menu entry for adding ignore and report patterns to the file view
- selected patterns to ignore or report will now be added escaped as regular expression
- added the report patterns menu to the toolbar
- increased status message view time
- the text fields to filter or add ignore and report patterns now have a clear button 
- added releases for Ubuntu Linux 18.04 and 18.10

## 16.06.0
- fixed a redraw error on OS X in the settings dialog

## 16.04.5
- renamed the *Remove ignored patterns from text* button to *Filter ignored 
  patterns from text* and changed the icon
- improved reporting and stability when downloading log files

## 16.04.4
- a connection test to your eZ Publish server is now performed in the settings

## 16.04.3
- downloaded and decompressed `gz`-files are now correctly renamed
- you can now select whether you want a prefix for downloaded log files or 
  not for each log file source in the settings
- log files are now also getting downloaded when double clicking on them
- the active log file source will be selected when the settings dialog is opened
- the modification time of the remote and local files will now be viewed
  in the file tables
- different row background colors will be used to indicate downloading files
- downloaded files will now automatically get a suffix if they already exist 
  locally
- the optional prefix for downloaded files was reduced to the log file source
  name
- the file size and modification time of the files in the evaluation list is 
  now shown
- files that are removed from the local log file table are now also removed 
  from the evaluation file list widget
- fixed a possible crash when downloading log files while there is a network 
  connection error and added better network error messages
- added a context menu to the local log file table to remove log files and 
  add log files to the evaluation list 
- added a context menu to the evaluation log list to remove log files 

## 16.04.2
- **switched LogAnalyzer release versioning** to a more *rolling release 
  style*, so it doesn't get confused with semantic versioning
    - `<year of release>.<month of release>.<release number in the month>` 
- implemented extracting of gz-compressed log files 

## 0.8
- eZ Publish passwords will now be stored encrypted (but not very securely)
- some styling changes
- remote log files can now be selected in a table and downloaded from there
    - it's possible to download multiple log files at the same time
    - a download indicator (percentage) will be shown for log file in the table
    - the file size of each remote and local log file is viewed in their tables
    - the remote and local log files in the table can be sorted by name and 
      file size
    - local log files can be removed from the file system in the table by 
      pressing <kbd>Del</kbd>
        - Note: removed local files are not yet removed from the global log 
          file list (this list will be a table too in the future) 
    - Note: log files are not yet compressed before downloading

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
