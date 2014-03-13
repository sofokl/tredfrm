#ifndef DISKID32_H
#define DISKID32_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <winioctl.h>

//  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  16

//  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported

//  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define PRINTING_TO_CONSOLE_ALLOWED
//#define TITLE   L"DiskId32"


namespace diskid {

      typedef struct _GETVERSIONINPARAMS {
            UCHAR    bVersion;               // Binary driver version.
            UCHAR    bRevision;              // Binary driver revision.
            UCHAR    bReserved;              // Not used.
            UCHAR    bIDEDeviceMap;          // Bit map of IDE devices.
            ULONG   fCapabilities;          // Bit mask of driver capabilities.
            ULONG   dwReserved[4];          // For future use.
    } GETVERSIONINPARAMS, *PGETVERSIONINPARAMS, *LPGETVERSIONINPARAMS;

    //  IDE registers
    typedef struct _IDEREGS
    {
       BYTE bFeaturesReg;       // Used for specifying SMART "commands".
       BYTE bSectorCountReg;    // IDE sector count register
       BYTE bSectorNumberReg;   // IDE sector number register
       BYTE bCylLowReg;         // IDE low order cylinder value
       BYTE bCylHighReg;        // IDE high order cylinder value
       BYTE bDriveHeadReg;      // IDE drive/head register
       BYTE bCommandReg;        // Actual IDE command.
       BYTE bReserved;          // reserved for future use.  Must be zero.
    } IDEREGS, *PIDEREGS, *LPIDEREGS;

    //  SENDCMDINPARAMS contains the input parameters for the
    //  Send Command to Drive function.
    typedef struct _SENDCMDINPARAMS
    {
        DWORD     cBufferSize;   //  Buffer size in bytes
        IDEREGS   irDriveRegs;   //  Structure with drive register values.
        BYTE bDriveNumber;       //  Physical drive number to send
                                 //  command to (0,1,2,3).
        BYTE bReserved[3];       //  Reserved for future expansion.
        DWORD     dwReserved[4]; //  For future use.
        BYTE      bBuffer[1];    //  Input buffer.
    } SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

    // Status returned from driver
    typedef struct _DRIVERSTATUS
    {
        BYTE  bDriverError;     //  Error code from driver, or 0 if no error.
        BYTE  bIDEStatus;       //  Contents of IDE Error register.
                                //  Only valid when bDriverError is SMART_IDE_ERROR.
        BYTE  bReserved[2];     //  Reserved for future expansion.
        DWORD  dwReserved[2];   //  Reserved for future expansion.
    } DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

    // Structure returned by PhysicalDrive IOCTL for several commands
    typedef struct _SENDCMDOUTPARAMS
    {
        DWORD         cBufferSize;   //  Size of bBuffer in bytes
        DRIVERSTATUS  DriverStatus;  //  Driver status structure.
        BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
    } SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

    //
    // Types of queries
    //

    typedef enum _STORAGE_QUERY_TYPE {
        PropertyStandardQuery = 0,          // Retrieves the descriptor
        PropertyExistsQuery,                // Used to test whether the descriptor is supported
        PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
        PropertyQueryMaxDefined     // use to validate the value
    } STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

    //
    // define some initial property id's
    //

    typedef enum _STORAGE_PROPERTY_ID {
        StorageDeviceProperty = 0,
        StorageAdapterProperty
    } STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

    //
    // Query structure - additional parameters for specific queries can follow
    // the header
    //

    typedef struct _STORAGE_PROPERTY_QUERY {

        //
        // ID of the property being retrieved
        //

        STORAGE_PROPERTY_ID PropertyId;

        //
        // Flags indicating the type of query being performed
        //

        STORAGE_QUERY_TYPE QueryType;

        //
        // Space for additional parameters if necessary
        //

        UCHAR AdditionalParameters[1];

    } STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

    typedef struct _DISK_GEOMETRY_EX {
          DISK_GEOMETRY  Geometry;
          LARGE_INTEGER  DiskSize;
          UCHAR  Data[1];
    } DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;

    typedef struct _STORAGE_DEVICE_DESCRIPTOR {

        //
        // Sizeof(STORAGE_DEVICE_DESCRIPTOR)
        //

        ULONG Version;

        //
        // Total size of the descriptor, including the space for additional
        // data and id strings
        //

        ULONG Size;

        //
        // The SCSI-2 device type
        //

        UCHAR DeviceType;

        //
        // The SCSI-2 device type modifier (if any) - this may be zero
        //

        UCHAR DeviceTypeModifier;

        //
        // Flag indicating whether the device's media (if any) is removable.  This
        // field should be ignored for media-less devices
        //

        BOOLEAN RemovableMedia;

        //
        // Flag indicating whether the device can support mulitple outstanding
        // commands.  The actual synchronization in this case is the responsibility
        // of the port driver.
        //

        BOOLEAN CommandQueueing;

        //
        // Byte offset to the zero-terminated ascii string containing the device's
        // vendor id string.  For devices with no such ID this will be zero
        //

        ULONG VendorIdOffset;

        //
        // Byte offset to the zero-terminated ascii string containing the device's
        // product id string.  For devices with no such ID this will be zero
        //

        ULONG ProductIdOffset;

        //
        // Byte offset to the zero-terminated ascii string containing the device's
        // product revision string.  For devices with no such string this will be
        // zero
        //

        ULONG ProductRevisionOffset;

        //
        // Byte offset to the zero-terminated ascii string containing the device's
        // serial number.  For devices with no serial number this will be zero
        //

        ULONG SerialNumberOffset;

        //
        // Contains the bus type (as defined above) of the device.  It should be
        // used to interpret the raw device properties at the end of this structure
        // (if any)
        //

        STORAGE_BUS_TYPE BusType;

        //
        // The number of bytes of bus-specific data which have been appended to
        // this descriptor
        //

        ULONG RawPropertiesLength;

        //
        // Place holder for the first byte of the bus specific property data
        //

        UCHAR RawDeviceProperties[1];

    } STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;

    typedef struct _SYSTEMIDSDATA {
        long int system_id;
        char HardDriveSerialNumber [1024];
        char HardDriveModelNumber [1024];
    } SYSTEMIDSDATA, *PSYSTEMIDSDATA;

}

char *ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex, char* buf);
void PrintIdeInfo (int drive, DWORD diskdata [256]);
BOOL DoIDENTIFY (HANDLE, diskid::PSENDCMDINPARAMS, diskid::PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD);

diskid::SYSTEMIDSDATA sprintf_sn(bool getMC=false);

#endif // DISKID32_H
