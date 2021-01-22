// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestFileName.h"

#include "FileName.h"

#include <kpabase/SettingsData.h>

#include <QRegularExpression>

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
}

void KPATest::TestFileName::initTestCase()
{
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);
}

void KPATest::TestFileName::uninitialized()
{
    const DB::FileName nullFN;
    QVERIFY(nullFN.isNull());
}

void KPATest::TestFileName::absolute()
{
    QDir imageRoot { Settings::SettingsData::instance()->imageDirectory() };
    using DB::FileName;

    // empty filename
    const QRegularExpression imageRootWarning { QStringLiteral("Absolute filename is outside of image root:") };
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto emptyFN = FileName::fromAbsolutePath({});
    QVERIFY(emptyFN.isNull());

    // incorrect root
    const auto outsidePath = QStringLiteral("/notarealdirectory/test.jpg");
    QVERIFY2(!outsidePath.startsWith(tmpDir.path()), msgPreconditionFailed);
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto outsideFN = FileName::fromAbsolutePath(outsidePath);
    QVERIFY(outsideFN.isNull());

    // relative filename
    QDir cwd;
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto relativeFN = FileName::fromAbsolutePath(cwd.relativeFilePath(tmpDir.path() + QStringLiteral("/test.jpg")));
    QVERIFY(relativeFN.isNull());

    // correct filename
    const auto absoluteFilePath = imageRoot.absoluteFilePath(QStringLiteral("atest.jpg"));
    const auto correctFN = FileName::fromAbsolutePath(absoluteFilePath);
    QVERIFY(!correctFN.isNull());
    QCOMPARE(correctFN.absolute(), absoluteFilePath);
    QCOMPARE(correctFN.relative(), imageRoot.relativeFilePath(absoluteFilePath));
    QVERIFY(!correctFN.exists());
    QCOMPARE(correctFN.operator QUrl().toLocalFile(), absoluteFilePath);

    const auto sameFN = FileName::fromAbsolutePath(absoluteFilePath);
    QVERIFY(sameFN == correctFN);
    QVERIFY(sameFN != emptyFN);

    const auto relativeCorrectFN = FileName::fromRelativePath(imageRoot.relativeFilePath(absoluteFilePath));
    QVERIFY(!relativeCorrectFN.isNull());
    QCOMPARE(relativeCorrectFN.absolute(), correctFN.absolute());
    QCOMPARE(relativeCorrectFN.relative(), correctFN.relative());
    QVERIFY(relativeCorrectFN == correctFN);

    // existing correct filename
    QFile existingFile { imageRoot.absoluteFilePath(QStringLiteral("existing.jpg")) };
    QVERIFY2(existingFile.open(QIODevice::WriteOnly), msgPreconditionFailed);
    existingFile.close();
    QVERIFY2(existingFile.exists(), msgPreconditionFailed);
    const auto existingFN = FileName::fromAbsolutePath(existingFile.fileName());
    QVERIFY(!existingFN.isNull());
    QCOMPARE(existingFN.absolute(), existingFile.fileName());
    QCOMPARE(existingFN.relative(), imageRoot.relativeFilePath(existingFile.fileName()));
    QVERIFY(existingFN.exists());
    QVERIFY(correctFN != existingFN);

    // alphabetical order
    QVERIFY(correctFN < existingFN);
}

void KPATest::TestFileName::relative()
{
    QDir imageRoot { Settings::SettingsData::instance()->imageDirectory() };

    using DB::FileName;

    // empty filename
    QTest::ignoreMessage(QtWarningMsg, "Relative or absolute filename cannot be empty!");
    const auto emptyFN = FileName::fromRelativePath({});
    QVERIFY(emptyFN.isNull());

    // absolute filename
    const auto absoluteFilePath = imageRoot.absoluteFilePath(QStringLiteral("atest.jpg"));
    const QRegularExpression absolutePathWarning { QStringLiteral("Relative filename cannot start with '/':") };
    QTest::ignoreMessage(QtWarningMsg, absolutePathWarning);
    const auto absoluteFN = FileName::fromRelativePath(absoluteFilePath);
    QVERIFY(absoluteFN.isNull());

    // correct filename
    QDir cwd;
    const auto relativeFilePath = imageRoot.relativeFilePath(absoluteFilePath);
    const auto correctFN = FileName::fromRelativePath(relativeFilePath);
    QVERIFY(!correctFN.isNull());
    QCOMPARE(correctFN.absolute(), absoluteFilePath);
    QCOMPARE(correctFN.relative(), relativeFilePath);
    QVERIFY(!correctFN.exists());
    QCOMPARE(correctFN.operator QUrl().toLocalFile(), absoluteFilePath);

    const auto sameFN = FileName::fromRelativePath(relativeFilePath);
    QVERIFY(sameFN == correctFN);
    QVERIFY(sameFN != emptyFN);

    const auto absoluteCorrectFN = FileName::fromAbsolutePath(absoluteFilePath);
    QVERIFY(!absoluteCorrectFN.isNull());
    QCOMPARE(absoluteCorrectFN.absolute(), correctFN.absolute());
    QCOMPARE(absoluteCorrectFN.relative(), correctFN.relative());
    QVERIFY(absoluteCorrectFN == correctFN);

    // existing correct filename
    QFile existingFile { imageRoot.relativeFilePath(QStringLiteral("existing.jpg")) };
    QVERIFY2(existingFile.open(QIODevice::WriteOnly), msgPreconditionFailed);
    existingFile.close();
    QVERIFY2(existingFile.exists(), msgPreconditionFailed);
    const auto existingFN = FileName::fromRelativePath(existingFile.fileName());
    QVERIFY(!existingFN.isNull());
    QCOMPARE(existingFN.absolute(), imageRoot.absoluteFilePath(existingFile.fileName()));
    QCOMPARE(existingFN.relative(), existingFile.fileName());
    QVERIFY(existingFN.exists());
    QVERIFY(correctFN != existingFN);

    // alphabetical order
    QVERIFY(correctFN < existingFN);
}

QTEST_MAIN(KPATest::TestFileName)

// vi:expandtab:tabstop=4 shiftwidth=4:
