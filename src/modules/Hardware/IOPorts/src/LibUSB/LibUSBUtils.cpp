/* @file Набор утилит для работы с LibUSB. */

#include <QtCore/QDebug>
#include <QtCore/QMetaType>

#include <Hardware/IOPorts/LibUSBUtils.h>

#include "Hardware/Common/CodecDescriptions.h"
#include "Hardware/Common/DataPointer.h"
#include "Hardware/Common/DeviceDataConstants.h"
#include "Hardware/Common/DeviceUtils.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"
#include "USBClassCodes.h"

using namespace ProtocolUtils;

namespace CLibUSBUtils {
const char Locale[] = "ru";
} // namespace CLibUSBUtils

namespace LibUSBUtils {
//--------------------------------------------------------------------------------
libusb_context *getContext(ILog *aLog) {
    static SPData<libusb_context *> context;

    LIB_USB_CALL_LOG(aLog, libusb_setlocale, CLibUSBUtils::Locale);

    if (!context && !LIB_USB_CALL_LOG(aLog, libusb_init, &context)) {
        context = nullptr;
    }

    return context;
}

//--------------------------------------------------------------------------------
void releaseContext(ILog *aLog) {
    libusb_context *context = LibUSBUtils::getContext(aLog);

    if (context) {
        // если не удалять контекст - повторно он проинициализируется корректно. Если удалять -
        // потом будет падать. libusb_exit(context);
        context = nullptr;
    }
}

//--------------------------------------------------------------------------------
libusb_device **getDeviceList(ssize_t &aSize, bool aForce) {
    static SPData<libusb_device **> deviceList;
    static SPData<int> size;

    if (deviceList && aForce) {
        // параметр булевский -> отпустить/держать параметры/контексты девайсов.
        // если не удалять устройство - повторно оно создастся корректно. Если удалять - потом будет
        // падать. libusb_free_device_list(deviceList, 1);
        deviceList = nullptr;
    }

    if (!deviceList || aForce) {
        libusb_context *context = getContext();

        if (!context) {
            return nullptr;
        }

        size = libusb_get_device_list(context, &deviceList);
    }

    // TODO: Find proper solution for SPData<int> to ssize_t assignment. The original code was:
    // aSize = size;
    aSize = static_cast<int>(size);

    return deviceList;
}

//--------------------------------------------------------------------------------
bool getDeviceList(TDeviceList &aList, bool aForce) {
    ssize_t size = 0;
    libusb_device **deviceList = getDeviceList(size, aForce);

    if (!deviceList) {
        return false;
    }

    aList.clear();

    for (int i = 0; i < size; ++i) {
        aList << deviceList[i];
    }

    return true;
}

//--------------------------------------------------------------------------------
void releaseDeviceList() {
    ssize_t size = 0;
    libusb_device **deviceList = getDeviceList(size);

    if (deviceList) {
        // параметр булевский -> отпустить/держать параметры/контексты девайсов.
        // если не удалять устройство - повторно оно создастся корректно. Если удалять - потом будет
        // падать. libusb_free_device_list(deviceList, 1);
        deviceList = nullptr;
    }
}

//--------------------------------------------------------------------------------
void addDescriptorData(libusb_device_handle *aHandle, uint8_t aDescriptor, QVariant &aData) {
    QVector<unsigned char> data;
    data.fill(ASCII::NUL, CLibUSBUtils::DescriptorDataSize);
    aData.clear();

    if ((aDescriptor != 0U) &&
        (libusb_get_string_descriptor_ascii(
             aHandle, aDescriptor, data.data(), CLibUSBUtils::DescriptorDataSize) != 0)) {
        aData = ProtocolUtils::clean(QString((char *)data.data()));
    }
}

//--------------------------------------------------------------------------------
QString getPropertyLog(const QVariantMap &aData) {
    return getPropertyLog(CLibUSB::TDeviceDataList() << aData);
}

//--------------------------------------------------------------------------------
QString getPropertyLog(const CLibUSB::TDeviceDataList &aList, int aIndex) {
    QString result;

    for (const auto &data : aList) {
        TDeviceData deviceData;
        QStringList complexKeys;

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it->typeId() == QMetaType::User) {
                complexKeys << it.key();
                auto complexList = data[it.key()].value<CLibUSB::TDeviceDataList>();
            } else {
                deviceData.insert(it.key(), it->toString());
            }
        }

        result += "\n" + DeviceUtils::getPartDeviceData(deviceData, true, aIndex);

        for (const auto &complexKey : complexKeys) {
            auto complexList = data[complexKey].value<CLibUSB::TDeviceDataList>();

            if (!complexList.isEmpty()) {
                int index = aIndex;
                result += "\n" + QString(aIndex, ASCII::TAB) + QString("%1 :").arg(complexKey);
                result += getPropertyLog(complexList, ++aIndex);
                aIndex = index;
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
bool getDevicesProperties(CLibUSB::TDeviceProperties &aDeviceProperties, bool aForce) {
    aDeviceProperties.clear();
    TDeviceList deviceList;

    if (!getContext() || !getDeviceList(deviceList, aForce)) {
        return false;
    }

    for (auto *device : deviceList) {
        aDeviceProperties.insert(device, getDevicesProperties(device));
    }

    return true;
}

//--------------------------------------------------------------------------------
CLibUSB::SDeviceProperties getDevicesProperties(libusb_device *aDevice) {
    CLibUSB::SDeviceProperties deviceProperties;
    QVariantMap &deviceData = deviceProperties.deviceData;

    uint8_t bus = libusb_get_bus_number(aDevice);
    uint8_t port = libusb_get_port_number(aDevice);
    uint8_t address = libusb_get_device_address(aDevice);

    deviceData.insert(DeviceUSBData::BusNumber, bus);
    deviceData.insert(DeviceUSBData::PortNumber, port);
    deviceData.insert(DeviceUSBData::Address, address);
    deviceProperties.portData =
        QString("Port %1 on bus %2 (address %3)").arg(port).arg(bus).arg(address);

    libusb_device_descriptor deviceDescriptor = {0};

    if (LIB_USB_CALL_DEBUG(libusb_get_device_descriptor, aDevice, &deviceDescriptor)) {
        getDeviceDescriptorData(aDevice, deviceDescriptor, deviceProperties);
    }

    return deviceProperties;
}

//--------------------------------------------------------------------------------
void getDeviceDescriptorData(libusb_device *aDevice,
                             const libusb_device_descriptor &aDeviceDescriptor,
                             CLibUSB::SDeviceProperties &aDeviceProperties) {
    QVariantMap &deviceData = aDeviceProperties.deviceData;

    deviceData.insert(DeviceUSBData::Specification, doubleBCD2String(aDeviceDescriptor.bcdUSB));
    deviceData.insert(DeviceUSBData::FirmwareVersion,
                      doubleBCD2String(aDeviceDescriptor.bcdDevice));

    QString deviceCode = QString("%1-%2-%3")
                             .arg(aDeviceDescriptor.bDeviceClass)
                             .arg(aDeviceDescriptor.bDeviceSubClass)
                             .arg(aDeviceDescriptor.bDeviceProtocol);
    deviceData.insert(DeviceUSBData::Code, deviceCode);

    QString deviceDescription = USB::ClassData.getData(aDeviceDescriptor.bDeviceClass,
                                                       aDeviceDescriptor.bDeviceSubClass,
                                                       aDeviceDescriptor.bDeviceProtocol);
    deviceData.insert(DeviceUSBData::Description, deviceDescription);

    deviceData.insert(DeviceUSBData::EP0PacketSize, aDeviceDescriptor.bMaxPacketSize0);

    aDeviceProperties.VID = aDeviceDescriptor.idVendor;
    aDeviceProperties.PID = aDeviceDescriptor.idProduct;

    deviceData.insert(CHardwareUSB::VID, toHexLog(aDeviceProperties.VID));
    deviceData.insert(CHardwareUSB::PID, toHexLog(aDeviceProperties.PID));

    deviceData.insert(DeviceUSBData::Vendor, deviceData[CHardwareUSB::VID]);
    deviceData.insert(DeviceUSBData::Product, deviceData[CHardwareUSB::PID]);

    deviceData.insert(DeviceUSBData::ConfigAmount, aDeviceDescriptor.bNumConfigurations);

    libusb_device_handle *deviceHandle = nullptr;

    if (LIB_USB_CALL_DEBUG(libusb_open, aDevice, &deviceHandle) && (deviceHandle != nullptr)) {
        addDescriptorData(
            deviceHandle, aDeviceDescriptor.iManufacturer, deviceData[DeviceUSBData::Vendor]);
        addDescriptorData(
            deviceHandle, aDeviceDescriptor.iProduct, deviceData[DeviceUSBData::Product]);
        addDescriptorData(
            deviceHandle, aDeviceDescriptor.iSerialNumber, deviceData[CDeviceData::SerialNumber]);

        libusb_close(deviceHandle);
    }

    TLibUSBDataList configData = getConfigData(aDevice, aDeviceDescriptor);

    if (!deviceData.contains(DeviceUSBData::ConfigAmount)) {
        deviceData.insert(DeviceUSBData::ConfigAmount, configData.size());
    }

    deviceData.insert(DeviceUSBData::ConfigData, QVariant::fromValue<TLibUSBDataList>(configData));

    if (deviceHandle && (aDeviceDescriptor.bcdUSB >= CLibUSBUtils::USB2_01)) {
        deviceData.insert(DeviceUSBData::BOSData, getBOSData(deviceHandle));
    }

    for (auto it = deviceData.begin(); it != deviceData.end(); ++it) {
        if (it->isNull()) {
            deviceData.remove((it++).key());
        }
    }

    QVariantMap data(deviceData);

    if (getDataFrom_Map(data, DeviceUSBData::ConfigData) &&
        getDataFrom_Map(data, DeviceUSBData::Config::InterfaceData)) {
        int endpointAmounts = data[DeviceUSBData::Config::Interface::EndpointAmount].toInt();

        for (int i = 0; i < endpointAmounts; ++i) {
            namespace InterfaceData = DeviceUSBData::Config::Interface;

            if (getDataFrom_Map(data, InterfaceData::EndpointData, false)) {
                QVariantMap epData =
                    data[InterfaceData::EndpointData].value<CLibUSB::TDeviceDataList>()[i];

                namespace EndpointData = DeviceUSBData::Config::Interface::Endpoint;

                QByteArray ep = getBufferFrom_String(epData[EndpointData::Address].toString());
                int maxPacketSize = epData[EndpointData::MaxPacketSize].toInt();
                int pollingInterval = epData[EndpointData::PollingInterval].toInt();
                QString transferTypeData = epData[EndpointData::TransferType].toString();
                libusb_transfer_type transferType =
                    CLibUSBUtils::TransferTypeDescriptions.key(transferTypeData);

                CLibUSB::SEndPoint &deviceEP = ((ep[0] & LIBUSB_ENDPOINT_DIR_MASK) != 0)
                                                   ? aDeviceProperties.deviceToHost
                                                   : aDeviceProperties.hostToDevice;
                deviceEP = CLibUSB::SEndPoint(transferType, ep[0], maxPacketSize, pollingInterval);
                deviceEP.processIO = (transferType == LIBUSB_TRANSFER_TYPE_BULK)
                                         ? &libusb_bulk_transfer
                                         : &libusb_interrupt_transfer;
            }
        }
    }
}

//--------------------------------------------------------------------------------
QVariantMap getBOSData(libusb_device_handle *aDeviceHandle) {
    QVariantMap result;

    struct libusb_bos_descriptor *bos = nullptr;

    if (LIB_USB_CALL_DEBUG(libusb_get_bos_descriptor, aDeviceHandle, &bos)) {
        if (bos->dev_capability[0]->bDevCapabilityType == LIBUSB_BT_USB_2_0_EXTENSION) {
            libusb_usb_2_0_extension_descriptor *usb20Extension = nullptr;

            if (LIB_USB_CALL_DEBUG(libusb_get_usb_2_0_extension_descriptor,
                                   getContext(),
                                   bos->dev_capability[0],
                                   &usb20Extension)) {
                result.insert(DeviceUSBData::BOS::Capability, "USB 2.0");
                result.insert(DeviceUSBData::BOS::Capability2_0::Attributes,
                              toHexLog(usb20Extension->bmAttributes));

                libusb_free_usb_2_0_extension_descriptor(usb20Extension);
            }
        } else if (bos->dev_capability[0]->bDevCapabilityType ==
                   LIBUSB_BT_SS_USB_DEVICE_CAPABILITY) {
            libusb_ss_usb_device_capability_descriptor *deviceCapability = nullptr;

            if (LIB_USB_CALL_DEBUG(libusb_get_ss_usb_device_capability_descriptor,
                                   getContext(),
                                   bos->dev_capability[0],
                                   &deviceCapability)) {
                result.insert(DeviceUSBData::BOS::Capability, "USB 3.0");
                result.insert(DeviceUSBData::BOS::Capability3_0::Attributes,
                              toHexLog(deviceCapability->bmAttributes));
                result.insert(DeviceUSBData::BOS::Capability3_0::SpeedSupported,
                              CLibUSBUtils::SpeedDescriptions[deviceCapability->wSpeedSupported]);
                result.insert(DeviceUSBData::BOS::Capability3_0::FunctionalitySupport,
                              deviceCapability->bFunctionalitySupport);
                result.insert(DeviceUSBData::BOS::Capability3_0::U1ExitLatency,
                              deviceCapability->bU1DevExitLat);
                result.insert(DeviceUSBData::BOS::Capability3_0::U2ExitLatency,
                              deviceCapability->bU2DevExitLat);

                libusb_free_ss_usb_device_capability_descriptor(deviceCapability);
            }
        }

        libusb_free_bos_descriptor(bos);
    }

    return result;
}

//--------------------------------------------------------------------------------
TLibUSBDataList getConfigData(libusb_device *aDevice,
                              const libusb_device_descriptor &aDeviceDescriptor) {
    TLibUSBDataList result;

    for (uint8_t i = 0; i < aDeviceDescriptor.bNumConfigurations; ++i) {
        libusb_config_descriptor *config = nullptr;

        if (LIB_USB_CALL_DEBUG(libusb_get_config_descriptor, aDevice, i, &config)) {
            namespace ConfigData = DeviceUSBData::Config;

            QVariantMap configData;

            configData.insert(ConfigData::InterfaceAmount, config->bNumInterfaces);
            configData.insert(ConfigData::Index, config->iConfiguration);
            configData.insert(ConfigData::Value, toHexLog(config->bConfigurationValue));
            configData.insert(ConfigData::Attributes, toHexLog(config->bmAttributes));
            configData.insert(ConfigData::MaxPower, config->MaxPower /* + " m_A"*/);

            // TODO: добавить параметры
            // TODO: уточнить MaxPower по поводу единиц измерения. Сила тока тут обозначается как
            // power, это не мощность.
            /*
            // Extra descriptors. If libusb encounters unknown configuration descriptors, it will
            store them here, should you wish to parse them. const unsigned char *extra;

            // Length of the extra descriptors, in bytes. Must be non-negative.
            int extra_length;

            // Maximum power consumption of the USB device from this bus in this configuration when
            the device is fully operation. Expressed
            // in units of 2 m_A when the device is operating in high-speed mode and
            // in units of 8 m_A when the device is operating in super-speed mode.
            */

            TLibUSBDataList interfaceData = getInterfaceData(config);
            configData.insert(ConfigData::InterfaceData,
                              QVariant::fromValue<TLibUSBDataList>(interfaceData));

            result << configData;

            libusb_free_config_descriptor(config);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
TLibUSBDataList getInterfaceData(libusb_config_descriptor *config) {
    TLibUSBDataList result;

    for (int i = 0; i < config->bNumInterfaces; ++i) {
        libusb_interface configInterface = config->interface[i];

        for (int j = 0; j < configInterface.num_altsetting; ++j) {
            namespace InterfaceData = DeviceUSBData::Config::Interface;

            libusb_interface_descriptor interface = configInterface.altsetting[j];
            QVariantMap interfaceData;

            interfaceData.insert(InterfaceData::Number, interface.bInterfaceNumber);
            interfaceData.insert(InterfaceData::EndpointAmount, interface.bNumEndpoints);
            interfaceData.insert(InterfaceData::Index, interface.iInterface);
            interfaceData.insert(InterfaceData::AlternateSetting,
                                 toHexLog(interface.bAlternateSetting));

            QString interfaceCode = QString("%1-%2-%3")
                                        .arg(interface.bInterfaceClass)
                                        .arg(interface.bInterfaceSubClass)
                                        .arg(interface.bInterfaceProtocol);
            interfaceData.insert(InterfaceData::Code, interfaceCode);

            QString interfaceDescription = USB::ClassData.getData(interface.bInterfaceClass,
                                                                  interface.bInterfaceSubClass,
                                                                  interface.bInterfaceProtocol);
            interfaceData.insert(InterfaceData::Description, interfaceDescription);

            // TODO: добавить параметры
            /*
            // Extra descriptors. If libusb encounters unknown interface descriptors, it will store
            them here, should you wish to parse them. const unsigned char *extra;

            // Length of the extra descriptors, in bytes. Must be non-negative.
            int extra_length;
            */

            TLibUSBDataList endpointData = getEndpointData(interface);
            interfaceData.insert(InterfaceData::EndpointData,
                                 QVariant::fromValue<TLibUSBDataList>(endpointData));

            result << interfaceData;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
TLibUSBDataList getEndpointData(const libusb_interface_descriptor &aInterface) {
    TLibUSBDataList result;

    for (int i = 0; i < aInterface.bNumEndpoints; ++i) {
        namespace EndpointData = DeviceUSBData::Config::Interface::Endpoint;

        libusb_endpoint_descriptor endpoint = aInterface.endpoint[i];
        QVariantMap endpointData;

        uint8_t attributes = endpoint.bmAttributes;
        endpointData.insert(EndpointData::TransferType,
                            CLibUSBUtils::TransferTypeDescriptions[attributes]);

        libusb_transfer_type transferType = CLibUSBUtils::transferType(attributes);

        if (transferType == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS) {
            endpointData.insert(EndpointData::IsoSyncType,
                                CLibUSBUtils::IsoSyncTypeDescriptions[attributes]);
            endpointData.insert(EndpointData::IsoUsageType,
                                CLibUSBUtils::IsoUsageTypeDescriptions[attributes]);
        }

        endpointData.insert(EndpointData::Address, toHexLog(endpoint.bEndpointAddress));
        endpointData.insert(EndpointData::Attributes, toHexLog(endpoint.bmAttributes));
        endpointData.insert(EndpointData::MaxPacketSize, endpoint.wMaxPacketSize);
        endpointData.insert(EndpointData::PollingInterval, endpoint.bInterval);
        endpointData.insert(EndpointData::SyncRefreshRate,
                            endpoint.bRefresh); // только для аудио-устройств
        endpointData.insert(EndpointData::SynchAddress, toHexLog(endpoint.bSynchAddress));

        endpointData.insert(EndpointData::CompanionAmount, endpoint.extra_length);

        // TODO: добавить и уточнить параметры
        /*
        // The address of the endpoint described by this descriptor.
        // Bits 0:3 are the endpoint number.
        // Bits 4:6 are reserved.
        // Bit 7 indicates direction, see \ref libusb_endpoint_direction.
        uint8_t  bEndpointAddress;

        // Attributes which apply to the endpoint when it is configured using the
        bConfigurationValue.
        // Bits 0:1 determine the transfer type and correspond to \ref libusb_transfer_type.
        // Bits 2:3 are only used for isochronous endpoints and correspond to \ref
        libusb_iso_sync_type.
        // Bits 4:5 are also only used for isochronous endpoints and correspond to \ref
        libusb_iso_usage_type.
        // Bits 6:7 are reserved.
        uint8_t  this->m_Attributes;

        // Extra descriptors. If libusb encounters unknown endpoint descriptors, it will store them
        here, should you wish to parse them. const unsigned char *extra;

        // Length of the extra descriptors, in bytes. Must be non-negative.
        int extra_length;
        */

        TLibUSBDataList epCompanionData = getEPCompanionData(endpoint);
        endpointData.insert(EndpointData::CompanionData,
                            QVariant::fromValue<TLibUSBDataList>(epCompanionData));

        result << endpointData;
    }

    return result;
}

//--------------------------------------------------------------------------------
TLibUSBDataList getEPCompanionData(const libusb_endpoint_descriptor &aEndpoint) {
    TLibUSBDataList result;

    for (int i = 0; i < aEndpoint.extra_length;) {
        if (LIBUSB_DT_SS_ENDPOINT_COMPANION == aEndpoint.extra[i + 1]) {
            struct libusb_ss_endpoint_companion_descriptor *epCompanionDescriptor = nullptr;

            if (LIB_USB_CALL_DEBUG(libusb_get_ss_endpoint_companion_descriptor,
                                   getContext(),
                                   &aEndpoint,
                                   &epCompanionDescriptor)) {
                namespace CompanionEPData = DeviceUSBData::Config::Interface::Endpoint::Companion;

                QVariantMap companionEPData;
                companionEPData.insert(CompanionEPData::MaxBurstPacketAmount,
                                       epCompanionDescriptor->bMaxBurst);
                companionEPData.insert(CompanionEPData::Attributes,
                                       toHexLog(epCompanionDescriptor->bmAttributes));
                companionEPData.insert(CompanionEPData::BytesPerInterval,
                                       epCompanionDescriptor->wBytesPerInterval);

                result << companionEPData;

                // TODO: уточнить параметр
                /*
                // In bulk EP: bits 4:0 represents the maximum number of streams the EP supports.
                // In isochronous EP: bits 1:0 represents the Mult - a zero based value that
                determines the maximum number of packets within a service interval uint8_t
                this->m_Attributes;
                */
            }

            libusb_free_ss_endpoint_companion_descriptor(epCompanionDescriptor);
        }

        i += aEndpoint.extra[i];
    }

    return result;
}

//--------------------------------------------------------------------------------
TResult logAnswer(const QString &aFunctionName, int aResult, ILog *aLog) {
    if (LIB_USB_SUCCESS(aResult) || (aResult == LIBUSB_ERROR_TIMEOUT)) {
        return CommandResult::OK;
    }

    QString errorName = libusb_error_name(aResult);
    QString errorDescription = QString::fromUtf8(libusb_strerror(libusb_error(aResult)));
    QString log = QString("Failed to call %1, error = %2 (%3, %4)")
                      .arg(aFunctionName)
                      .arg(aResult)
                      .arg(errorName)
                      .arg(errorDescription);

    if (aLog) {
        aLog->write(LogLevel::Error, log);
    } else {
        // QTextCodec::setCodecForCStrings(codec); // Removed: not available in Qt 5.0+; use UTF-8
        // everywhere.

        qDebug() << log;
    }

    return aResult;
}

//--------------------------------------------------------------------------------
template <class T> QString doubleBCD2String(T aData) {
    int size = sizeof(aData);
    QString result = QString::number(aData, 16).rightJustified(size * 2, ASCII::Zero);
    QString fraction = result.right(size);
    int index = qMax(fraction.indexOf(ASCII::Zero), 1);

    return QString("%1.%2").arg(result.left(size).toUInt()).arg(fraction.left(index));
}

//--------------------------------------------------------------------------------
bool getDataFrom_Map(QVariantMap &aData, const QString &aKey, bool aSetNextData) {
    if (!aData.contains(aKey) ||
        !aData[aKey].isValid() && (aData[aKey].typeId() != QMetaType::User)) {
        return false;
    }

    auto deviceDataList = aData[aKey].value<CLibUSB::TDeviceDataList>();

    if (deviceDataList.isEmpty()) {
        return false;
    }

    if (aSetNextData) {
        aData = deviceDataList[0];
    }

    return true;
};

} // namespace LibUSBUtils
//--------------------------------------------------------------------------------
