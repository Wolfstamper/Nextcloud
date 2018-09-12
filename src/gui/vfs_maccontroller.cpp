/*
 * Copyright (C) 2018 by AMCO
 * Copyright (C) 2018 by Jesús Deloya <jdeloya_ext@amco.mx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <QMessageBox>
#include <QApplication>

#include "vfs_maccontroller.h"
#include "vfs_mac.h"

#include <AvailabilityMacros.h>


void VfsMacController::mountFailed(QVariantMap userInfo)
{
    qDebug() << "Got mountFailed notification.";
    
    qDebug() << "kGMUserFileSystem Error code: " << userInfo.value("code") << ", userInfo=" << userInfo.value("localizedDescription");
    
    QMessageBox alert;
    alert.setText(userInfo.contains("localizedDescription")?userInfo.value("localizedDescription").toString() : "Unknown error");
    alert.exec();
    
    QApplication::quit();
}

void VfsMacController::didMount(QVariantMap userInfo)
{
    qDebug() << "Got didMount notification.";
    
    QString mountPath = userInfo.value(kGMUserFileSystemMountPathKey).toString();
    /*QMessageBox alert;
    alert.setText(tr(QString("Mounted at: %1").arg(mountPath).toLatin1().data()));
    alert.exec();
     */
}

void VfsMacController::didUnmount(QVariantMap userInfo) {
    qDebug() << "Got didUnmount notification.";
    
    QApplication::quit();
}

void VfsMacController::unmount()
{
    fs_->unmount();
}

void VfsMacController::slotquotaUpdated(qint64 total, qint64 used)
{
    fs_->setTotalQuota(total);
    fs_->setUsedQuota(used);
}

VfsMacController::VfsMacController(QString rootPath, QString mountPath, OCC::AccountState *accountState, QObject *parent):QObject(parent), fs_(new VfsMac(rootPath, false, accountState, this))
{
    qi_ = new OCC::QuotaInfo(accountState, this);
    
    connect(qi_, &OCC::QuotaInfo::quotaUpdated, this, &VfsMacController::slotquotaUpdated);
    connect(fs_.data(), &VfsMac::FuseFileSystemDidMount, this, didMount);
    connect(fs_.data(), &VfsMac::FuseFileSystemMountFailed, this, mountFailed);
    connect(fs_.data(), &VfsMac::FuseFileSystemDidUnmount, this, didUnmount);
    
    qi_->setActive(true);
    
    QStringList options;
    
    QFileInfo icons(QCoreApplication::applicationDirPath() + "/../Resources/LoopbackFS.icns");
    QString volArg = QString("volicon=%1").arg(icons.canonicalFilePath());
    
    options.append(volArg);
    
    // Do not use the 'native_xattr' mount-time option unless the underlying
    // file system supports native extended attributes. Typically, the user
    // would be mounting an HFS+ directory through VfsMac, so we do want
    // this option in that case.
    options.append("native_xattr");
    
    options.append("volname=VfsMac");
    fs_->mountAtPath(mountPath, options);
}

/*VfsMacController::~VfsMacController()
{
    //fs_->unmount();
}*/
