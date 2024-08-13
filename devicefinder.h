#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>

class DeviceFinder : public QObject
{
    Q_OBJECT

public:
    explicit DeviceFinder(QObject *parent = nullptr);

private:
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;

    void addDevice(const QBluetoothDeviceInfo &device);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
};
