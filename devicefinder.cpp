#include "devicefinder.h"

#include <iostream>

DeviceFinder::DeviceFinder(QObject *parent) : QObject{parent}
{
    std::cout << "[INFO] Started device finder..." << std::endl;

    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    // device found and error callbacks
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceFinder::addDevice);
    connect(m_deviceDiscoveryAgent, static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(&QBluetoothDeviceDiscoveryAgent::error), this, &DeviceFinder::scanError);

    // scan finished callbacks
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceFinder::scanFinished);

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device) {
    // we only care about btle devices
    if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
        return;

    // this should probably be a list of valid addrs and names but this are the only ones i know are valid for now
    if (device.address() != QBluetoothAddress("FF:FF:10:FF:89:EC") || device.name() != "ELK-BLEDOM")
        return;

    std::cout << "[INFO] Got new Elk-Bledom device with addr: " << device.address().toString().toStdString()
              << ", name: " << device.name().toStdString()
              << std::endl;

    // look for the device attrs
    QLowEnergyController *m_control = QLowEnergyController::createCentral(device, this);

    connect(m_control, &QLowEnergyController::serviceDiscovered, this, [this, m_control](const QBluetoothUuid &newService) {
        // check if we have the correct service uuid
        // right now, this is the only service uuid i know is valid
        if (newService != QBluetoothUuid(QUuid("0000fff0-0000-1000-8000-00805f9b34fb")))
            return;

        QLowEnergyService *m_service = m_control->createServiceObject(QBluetoothUuid(QUuid("0000fff0-0000-1000-8000-00805f9b34fb")), this);
        if (!m_service)
            return;

        const QLowEnergyCharacteristic characteristic = m_service->characteristic(QBluetoothUuid(QUuid("0000fff3-0000-1000-8000-00805f9b34fb")));
        const QByteArray byteArray = QByteArray::fromRawData("\x7e\x07\x05\x03\xff\xff\xff\x10\xef", 9);
        m_service->writeCharacteristic(characteristic, byteArray);

        std::cout << "Service has the necessary UUID. Ready for use!" << std::endl;
    });

    // connect(m_control, &QLowEnergyController::discoveryFinished, this, [this](){});

    connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error), this,
        [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
            std::cout << "Cannot connect to the device." << std::endl;
    });

    connect(m_control, &QLowEnergyController::connected, this, [this, m_control]() {
        std::cout << "Device is connected, searching for services..." << std::endl;
        m_control->discoverServices();
    });

    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        std::cout << "Device got disconnected." << std::endl;
    });

    // connect to the device
    m_control->connectToDevice();
}

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error) {
    std::cout << "Error with the following type happened: ";
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::Error::NoError:                        { std::cout << "NoError" << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::InputOutputError:               { std::cout << "InputOutputError." << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::PoweredOffError:                { std::cout << "PoweredOffError." << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::InvalidBluetoothAdapterError:   { std::cout << "InvalidBluetoothAdapterError." << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::UnsupportedPlatformError:       { std::cout << "UnsupportedPlatformError." << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::UnsupportedDiscoveryMethod:     { std::cout << "UnsupportedDiscoveryMethod." << std::endl; break; }
    case QBluetoothDeviceDiscoveryAgent::Error::UnknownError:                   { std::cout << "UnknownError." << std::endl; break; }
    default: std::cout << "UnknownError." << std::endl;
    }
}

void DeviceFinder::scanFinished() {
    // TODO
}
