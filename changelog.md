# Changelog

### Version 0.3.0 (released on 15.08.2017)
 * Added XEP-0280: Message Carbons (#117) (@LNJ2)
 * Added XEP-0054/XEP-0153: VCard-based avatars (#73, #105, #119) (@LNJ2)
 * Added file storage for simply caching all avatars (@LNJ2)
 * New roster design - showing round avatars and last message (#118) (@LNJ2)
 * New chat page design - showing time, delivery status and round avatars (#123) (@LNJ2)
 * Switched to XMPP client library "gloox" (#114) (@LNJ2)
 * Rewritten most of the back-end for gloox and partialy also restructured it (#114) (@LNJ2)
 * (Re)written new LogHandler for gloox (Swiften had this already included) (#114) (@LNJ2)

### Version 0.2.3 (released on 19.06.2017)
Fixes:
 * LoginPage: Remove material shadow (#113) (@JBBgameich)
 * Kaidan was crashing since v0.2.2 when inserting a new message to the DB (@LNJ2)

### Version 0.2.2 (released on 19.06.2017)
Fixes:
 * RosterPage: Clear TextFields after closing AddContactSheet (#106) (@JBBgameich)

Unused back-end features:
 * RosterController: Save lastMessage for each contact (#108) (@LNJ2)
 * Add database versioning and conversion (#110) (@LNJ2)
 * Database: Add new roster row `avatarHash` (#112) (@LNJ2)

Misc:
 * CMake: Add feature summary (#109) (@LNJ2)

### Version 0.2.1 (released on 08.06.2017)
Fixes:
 * Roster page: Fixed style: Now has contour lines and a cool material effect (@LNJ2)

### Version 0.2.0 (released on 06.06.2017)
User interface:
 * GUI: Port to Kirigami 2 (#81) (@JBBgameich)
 * User Material/Green Theme per default (@LNJ2)
 * Login page: New design with diaspora* login option (#87) (@JBBgameich)
 * Chat page: Slightly improved design (@LNJ2)

New features:
 * Add Roster Editing (#84, #86) (@LNJ2, @JBBgameich)
 * Roster refreshes now automatically (#83) (@LNJ2)
 * Contacts are now sorted (@LNJ2)
 * Add unread message counters (#92, #101) (@LNJ2)
 * Add LibNotify-Linux notifications (#90) (@LNJ2)
 * Add custom JID resources (#82) (@LNJ2)
 * Add XEP-0184: Message Delivery Receipts (@LNJ2)
 * Disable stream compression by default (for HipChat/other server compatibility) (@LNJ2)

Fixes:
 * AboutPage: Fix possible closing of multiple pages (@LNJ2)
