// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>

#include <QXmppUtils.h>

#include "../src/PresenceCache.h"

Q_DECLARE_METATYPE(QList<QXmppPresence>)

class PresenceCacheTest : public QObject
{
	Q_OBJECT

private:
	Q_SLOT void initTestCase();
	Q_SLOT void presenceChangedSignal();
	Q_SLOT void presenceGetter_data();
	Q_SLOT void presenceGetter();
	Q_SLOT void idealResource_data();
	Q_SLOT void idealResource();

	void addBasicPresences();
	void addSimplePresence(const QString &jid,
		QXmppPresence::AvailableStatusType available = QXmppPresence::Online,
		const QString &status = {},
		QXmppPresence::Type type = QXmppPresence::Available);
	QXmppPresence simplePresence(const QString &jid,
		QXmppPresence::AvailableStatusType available = QXmppPresence::Online,
		const QString &status = {},
		QXmppPresence::Type type = QXmppPresence::Available);

	PresenceCache cache;
};

void PresenceCacheTest::initTestCase()
{
	qRegisterMetaType<PresenceCache::ChangeType>();
}

void PresenceCacheTest::presenceChangedSignal()
{
	cache.clear();
	enum { ChangeType, Jid, Resource };

	QXmppPresence p1;
	p1.setFrom("bob@kaidan.im/dev1");
	QXmppPresence p2(QXmppPresence::Unavailable);
	p2.setFrom("bob@kaidan.im/dev1");
	QXmppPresence p3(QXmppPresence::Unavailable);
	p3.setFrom("bob@kaidan.im/dev23");

	QSignalSpy spy(&cache, &PresenceCache::presenceChanged);
	auto signalCount = 0;

	QVERIFY(spy.isValid());
	QVERIFY(spy.isEmpty());

	cache.updatePresence(p1);
	QCOMPARE(spy.count(), ++signalCount);
	QCOMPARE(spy[0][ChangeType], QVariant::fromValue(PresenceCache::Connected));
	QCOMPARE(spy[0][Jid], "bob@kaidan.im");
	QCOMPARE(spy[0][Resource], "dev1");

	cache.updatePresence(p1);
	QCOMPARE(spy.count(), ++signalCount);
	QCOMPARE(spy[1][ChangeType], QVariant::fromValue(PresenceCache::Updated));

	cache.updatePresence(p1);
	QCOMPARE(spy.count(), ++signalCount);
	QCOMPARE(spy[2][ChangeType], QVariant::fromValue(PresenceCache::Updated));

	cache.updatePresence(p2);
	QCOMPARE(spy.count(), ++signalCount);
	QCOMPARE(spy[3][ChangeType], QVariant::fromValue(PresenceCache::Disconnected));

	// test unknown device disconnect
	cache.updatePresence(p3);
	QCOMPARE(spy.count(), signalCount);
}

void PresenceCacheTest::presenceGetter_data()
{
	QTest::addColumn<QString>("jid");
	QTest::addColumn<QString>("resource");
	QTest::addColumn<bool>("exists");

	QTest::newRow("bob1") << "bob@kaidan.im"
			      << "dev1" << true;
	QTest::newRow("bob2") << "bob@kaidan.im"
			      << "dev2" << true;
	QTest::newRow("alice1") << "alice@kaidan.im"
				<< "kdn1" << true;
	QTest::newRow("alice2") << "alice@kaidan.im"
				<< "kdn2" << true;
	QTest::newRow("unknwon presence") << "alice@kaidan.im"
					  << "device3" << false;
}

void PresenceCacheTest::presenceGetter()
{
	QFETCH(QString, jid);
	QFETCH(QString, resource);
	QFETCH(bool, exists);

	cache.clear();
	addBasicPresences();

	auto presence = cache.presence(jid, resource);
	QCOMPARE(presence.has_value(), exists);
	if (exists) {
		QCOMPARE(presence->from(), QStringView(u"%1/%2").arg(jid, resource));
	}
}

void PresenceCacheTest::idealResource_data()
{
	QTest::addColumn<QList<QXmppPresence>>("presences");
	QTest::addColumn<QString>("expectedResource");

#define ROW(name, presences, expectedResource) \
	QTest::newRow(name) << (presences) << QStringLiteral(expectedResource)

	ROW("DND",
		QList<QXmppPresence>() << simplePresence("a@b/1", QXmppPresence::Away)
				       << simplePresence("a@b/2", QXmppPresence::XA)
				       << simplePresence("a@b/3", QXmppPresence::Online)
				       << simplePresence("a@b/4", QXmppPresence::Chat)
				       << simplePresence("a@b/5", QXmppPresence::DND),
		"5");
	ROW("Chat",
		QList<QXmppPresence>() << simplePresence("a@b/1", QXmppPresence::Away)
				       << simplePresence("a@b/2", QXmppPresence::XA)
				       << simplePresence("a@b/3", QXmppPresence::Chat)
				       << simplePresence("a@b/4", QXmppPresence::Online),
		"3");
	ROW("XA",
		QList<QXmppPresence>() << simplePresence("a@b/1", QXmppPresence::XA)
				       << simplePresence("a@b/2", QXmppPresence::Away),
		"1");
	ROW("Away",
		QList<QXmppPresence>() << simplePresence("a@b/1", QXmppPresence::Away)
				       << simplePresence("a@b/2", QXmppPresence::XA),
		"1");
	ROW("status text [DND]",
		QList<QXmppPresence>()
			<< simplePresence("a@b/1", QXmppPresence::Away)
			<< simplePresence("a@b/2", QXmppPresence::XA)
			<< simplePresence("a@b/3", QXmppPresence::Online)
			<< simplePresence("a@b/4", QXmppPresence::Chat)
			<< simplePresence("a@b/5", QXmppPresence::DND)
			<< simplePresence("a@b/6", QXmppPresence::Chat, "my status")
			<< simplePresence("a@b/7", QXmppPresence::DND, "my status")
			<< simplePresence("a@b/8", QXmppPresence::Away, "my status"),
		"7");
	ROW("status text [XA]",
		QList<QXmppPresence>()
			<< simplePresence("a@b/1", QXmppPresence::Away)
			<< simplePresence("a@b/2", QXmppPresence::XA)
			<< simplePresence("a@b/3", QXmppPresence::XA, "my status")
			<< simplePresence("a@b/4", QXmppPresence::Away, "my status"),
		"3");

#undef ROW
}

void PresenceCacheTest::idealResource()
{
	QFETCH(QList<QXmppPresence>, presences);
	QFETCH(QString, expectedResource);

	cache.clear();
	for (const auto &presence : std::as_const(presences))
		cache.updatePresence(presence);

	const auto jid = QXmppUtils::jidToBareJid(presences.first().from());
	QCOMPARE(cache.pickIdealResource(jid), expectedResource);
}

void PresenceCacheTest::addBasicPresences()
{
	addSimplePresence("bob@kaidan.im/dev1");
	addSimplePresence("bob@kaidan.im/dev2");
	addSimplePresence("alice@kaidan.im/kdn1");
	addSimplePresence("alice@kaidan.im/kdn2");
}

void PresenceCacheTest::addSimplePresence(const QString &jid,
	QXmppPresence::AvailableStatusType available,
	const QString &status,
	QXmppPresence::Type type)
{
	cache.updatePresence(simplePresence(jid, available, status, type));
}

QXmppPresence PresenceCacheTest::simplePresence(const QString &jid,
	QXmppPresence::AvailableStatusType available,
	const QString &status,
	QXmppPresence::Type type)
{
	QXmppPresence p(type);
	p.setFrom(jid);
	p.setStatusText(status);
	p.setAvailableStatusType(available);
	return p;
}

QTEST_GUILESS_MAIN(PresenceCacheTest)
#include "PresenceCacheTest.moc"
