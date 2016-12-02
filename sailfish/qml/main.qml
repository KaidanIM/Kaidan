import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    cover: pageCover;
    initialPage: pageMenu;
    Component.onCompleted: {
        pageStack.push (pageContacts, { }, PageStackAction.Immediate);
    }

    Component { id: pageMenu; MenuPage { } }
    Component { id: pageCover; CoverPage { } }
    Component { id: pageContacts; ContactsPage { } }
    Component { id: pageConversations; ConversationsPage { } }
    Component { id: pagePreferences; PreferencesPage { } }
    Component { id: pageAccount; AccountPage { } }
    Component { id: pageMessaging; MessagingPage { } }
}


