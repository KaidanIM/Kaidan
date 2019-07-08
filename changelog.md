# Changelog

### Version 0.4.0 (released on Jul 08, 2019)

Build system:
 * Support for Android (ilyabizyaev)
 * Support for Ubuntu Touch (jbb)
 * Support for MacOS (ilyabizyaev)
 * Support for Windows (ilyabizyaev)
 * Support for iOS (ilyabizyaev)
 * Add KDE Flatpak (jbb)
 * Switch Android builds to CMake with ECM (ilyabizyaev)
 * Improve Linux AppImage build script (ilyabizyaev)
 * Add additional image formats in AppImage (jbb)

Features:
 * Show proper notifications using KNotifications (lnj)
 * Add settings page for changing passwords (jbb, lnj)
 * Add XEP-0352: Client State Indication (gloox/QXmpp) (lnj)
 * Add media/file (including GIFs) sharing (lnj, jbb)
 * Full back-end rewrite to QXmpp (lnj)
 * Implement XEP-0363: HTTP File Upload and UploadManager for QXmpp (lnj)
 * Use XEP-0280: Message Carbons from QXmpp (lnj)
 * Use XEP-0352: Client State Indication from QXmpp (lnj)
 * Check incoming messages for media links (lnj)
 * Implement XEP-0308: Last Message Correction (lnj, jbb)
 * Make attachments downloadable (lnj)
 * Implement XEP-0382: Spoiler messages (xavi)
 * Kaidan is now offline usable (lnj)
 * Kaidan is able to open xmpp: URIs (lnj)
 * New logo (ilyabizyaev)
 * Show presence information of contacts (lnj, melvo)
 * Add EmojiPicker from Spectral with search and favorites functionality (jbb, fazevedo)
 * Highlight links in chat and make links clickable (lnj)
 * New about dialog instead of the about page (ilyabizyaev)
 * Add image preview in chat and before sending (lnj)
 * Send messages on Enter, new line on Ctrl-Enter (ilyabizyaev)
 * 'Add contact' is now the main action on the contacts page (lnj)
 * Elide contact names and messages in roster (lnj)
 * Chat page redesign (ilyabizyaev)
 * Display passive notifications when trying to use online actions while offline (lnj)
 * Automatically reconnect on connection loss (lnj)
 * Contacts page: Display whether online in title (lnj)
 * Add different connection error messages (jbb)
 * Use QApplication when building with QWidgets (notmart)
 * Ask user to approve subscription requests (lnj)
 * Remove contact action: Make JIDs bold (lnj)
 * Add contact sheet: Ask for optional message to contact (lnj)
 * Add empty chat page with help notice to be displayed on start up (jbb)
 * Redesign log in page (sohnybohny)
 * Add Copy Invitaion URL action (jbb)
 * Add 'press and hold' functionality for messages context menu (jbb)
 * Add copy to clipboard function for messages (jbb)
 * Add mobile file chooser (jbb)
 * Highlight the currently opened chat on contacts page (lnj)
 * Remove predefined window sizes (lnj)
 * Use new Kirigami application header (nicofee)
 * Make images open externally when clicked (jbb)
 * Use QtQuickCompiler (jbb)
 * Display upload progress bar (lnj)
 * Add text+color avatars as fallback (lnj, jbb)
 * Remove diaspora log in option (lnj)

Misc:
 * Forget passwords on log out (lnj)
 * Append four random chars to resource (lnj)
 * Save passwords in base64 instead of clear text (lnj)
 * Generate the LICENSE file automatically with all git authors (lnj)
 * Store ubuntu touch builds as job artifacts (lnj)
 * Add GitLab CI integration (jbb)

Fixes:
 * Fix blocking of GUI thread while database interaction (lnj)
 * Fix TLS connection bug (lnj)
 * Don't send notifications when receiving own messages via. carbons (lnj)
 * Fix timezone bug of message timestamps (lnj)
 * Fix several message editing bugs (lnj)
 * Fix black icons (jbb)
 * Fix rich text labels in Plasma Mobile (lnj)
 * Small Plasma Mobile fixes (jbb)

### Version 0.3.2 (released on 25.11.2017)
 * Added AppImage build script (#138) (@JBBgameich)
 * Use relative paths to find resource files (#143) (@LNJ2)
 * Source directory is only used for resource files in debug builds (#146) (@LNJ2)

### Version 0.3.1 (released on 20.11.2017)
Fixes:
 * Fixed database creation errors (#135, #132) (@LNJ2)
 * ChatPage: Fixed recipient's instead of author's avatar displayed (#131, #137) (@LNJ2)

Misc:
 * Added Travis-CI integration (#133, #134, #136) (@JBBgameich)

Internationalization:
 * Added Malay translations (#129) (@MuhdNurHidayat)

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
