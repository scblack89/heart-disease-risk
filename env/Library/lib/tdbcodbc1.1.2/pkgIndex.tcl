# Index file to load the TDBC ODBC package.

if {[catch {package require Tcl 8.6}]} {
    return
}
package ifneeded tdbc::odbc 1.1.2 \
    "[list source [file join $dir tdbcodbc.tcl]]\;\
    [list load [file join $dir tdbcodbc112t.dll] tdbcodbc]"
