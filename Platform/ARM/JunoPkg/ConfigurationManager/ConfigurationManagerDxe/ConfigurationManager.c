/** @file
  Configuration Manager Dxe

  Copyright (c) 2017 - 2018, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
**/

#include <IndustryStandard/DebugPort2Table.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Protocol/AcpiTable.h>

#include <ArmPlatform.h>
#include <AcpiTableGenerator.h>
#include <Protocol/ConfigurationManagerProtocol.h>

#include "ConfigurationManager.h"
#include "Platform.h"

// AML Code Include files generated by iASL Compiler
#include <Dsdt.hex>
#include <SsdtJunoUsb.hex>
#include <SsdtUart.hex>
#include <SsdtPci.hex>

/** The platform configuration repository information.
*/
STATIC
EFI_PLATFORM_REPOSITORY_INFO ArmJunoPlatformRepositoryInfo = {
  /// Configuration Manager information
  { CONFIGURATION_MANAGER_REVISION, CFG_MGR_OEM_ID },

  // ACPI Table List
  {
    // FADT Table
    {
      EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_FADT),
      NULL
    },
    // GTDT Table
    {
      EFI_ACPI_6_2_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_GTDT),
      NULL
    },
    // MADT Table
    {
      EFI_ACPI_6_2_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_MADT),
      NULL
    },
    // SPCR Table
    {
      EFI_ACPI_6_2_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_SPCR),
      NULL
    },
    // DSDT Table
    {
      EFI_ACPI_6_2_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_DSDT),
      (EFI_ACPI_DESCRIPTION_HEADER*)dsdt_aml_code
    },
    // DBG2 Table
    {
      EFI_ACPI_6_2_DEBUG_PORT_2_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_DBG2),
      NULL
    },
    // SSDT Table describing the Juno USB
    {
      EFI_ACPI_6_2_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_SSDT),
      (EFI_ACPI_DESCRIPTION_HEADER*)ssdtjunousb_aml_code
    },
    // SSDT table describing the PL011 UART
    {
      EFI_ACPI_6_2_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_SSDT),
      (EFI_ACPI_DESCRIPTION_HEADER*)ssdtuart_aml_code
    },

    /* PCI MCFG Table
       PCIe is only available on Juno R1 and R2.
       Add the PCI table entries at the end of the table so that
       we can easily disable PCIe for Juno R0
    */
    {
      EFI_ACPI_6_2_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_MCFG),
      NULL
    },
    // SSDT table describing the PCI root complex
    {
      EFI_ACPI_6_2_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
      CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_SSDT),
      (EFI_ACPI_DESCRIPTION_HEADER*)ssdtpci_aml_code
    }
  },

  // Boot architecture information
  { EFI_ACPI_6_2_ARM_PSCI_COMPLIANT },      // BootArchFlags

  // Power management profile information
  { EFI_ACPI_6_2_PM_PROFILE_MOBILE },       // PowerManagement Profile

  /* GIC CPU Interface information
     GIC_ENTRY (CPUInterfaceNumber, Mpidr, PmuIrq, VGicIrq, EnergyEfficiency)
  */
  {
    GICC_ENTRY (2, GET_MPID (1, 0), 50, 25, 0),
    GICC_ENTRY (3, GET_MPID (1, 1), 54, 25, 0),
    GICC_ENTRY (4, GET_MPID (1, 2), 58, 25, 0),
    GICC_ENTRY (5, GET_MPID (1, 3), 62, 25, 0),

    GICC_ENTRY (0, GET_MPID (0, 0), 34, 25, 1),
    GICC_ENTRY (1, GET_MPID (0, 1), 38, 25, 1)
  },

  // GIC Distributor Info
  {
    0,                                      // UINT32  GicId
    FixedPcdGet64 (PcdGicDistributorBase),  // UINT64  PhysicalBaseAddress
    0,                                      // UINT32  SystemVectorBase
    2                                       // UINT8   GicVersion
  },

  // Generic Timer Info
  {
    // The physical base address for the counter control frame
    SYSTEM_COUNTER_BASE_ADDRESS,
    // The physical base address for the counter read frame
    SYSTEM_COUNTER_READ_BASE,
    // The secure PL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerSecIntrNum),
    // The secure PL1 timer flags
    GTDT_GTIMER_FLAGS,
    // The non-secure PL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerIntrNum),
    // The non-secure PL1 timer flags
    GTDT_GTIMER_FLAGS,
    // The virtual timer interrupt
    FixedPcdGet32 (PcdArmArchTimerVirtIntrNum),
    // The virtual timer flags
    GTDT_GTIMER_FLAGS,
    // The non-secure PL2 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerHypIntrNum),
    // The non-secure PL2 timer flags
    GTDT_GTIMER_FLAGS
  },

  // Watchdog Info
  {
    // The physical base address of the SBSA Watchdog control frame
    FixedPcdGet64 (PcdGenericWatchdogControlBase),
    // The physical base address of the SBSA Watchdog refresh frame
    FixedPcdGet64 (PcdGenericWatchdogRefreshBase),
    // The watchdog interrupt
    FixedPcdGet32 (PcdGenericWatchdogEl2IntrNum),
    // The watchdog flags
    SBSA_WATCHDOG_FLAGS
  },

  // SPCR Serial Port
  {
    FixedPcdGet64 (PcdSerialRegisterBase),            // UINT64  BaseAddress
    FixedPcdGet32 (PL011UartInterrupt),               // UINT32  Interrupt
    FixedPcdGet64 (PcdUartDefaultBaudRate),           // UINT64  BaudRate
    FixedPcdGet32 (PL011UartClkInHz),                 // UINT32  Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART  // UINT16  Port subtype
  },
  // Debug Serial Port
  {
    FixedPcdGet64 (PcdSerialDbgRegisterBase),         // UINT64  BaseAddress
    38,                                               // UINT32  Interrupt
    FixedPcdGet64 (PcdSerialDbgUartBaudRate),         // UINT64  BaudRate
    FixedPcdGet32 (PcdSerialDbgUartClkInHz),          // UINT32  Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART  // UINT16  Port subtype
  },

  // PCI Configuration Space Info
  {
    // The physical base address for the PCI segment
    FixedPcdGet64 (PcdPciConfigurationSpaceBaseAddress),
    // The PCI segment group number
    0,
    // The start bus number
    FixedPcdGet32 (PcdPciBusMin),
    // The end bus number
    FixedPcdGet32 (PcdPciBusMax)
  },

  // GIC Msi Frame Info
  {
    // The GIC MSI Frame ID
    0,
    // The Physical base address for the MSI Frame.
    ARM_JUNO_GIV2M_MSI_BASE,
    /* The GIC MSI Frame flags
       as described by the GIC MSI frame
       structure in the ACPI Specification.
    */
    0,
    // SPI Count used by this frame.
    127,
    // SPI Base used by this frame.
    224
  }
};

/** Initialize the platform configuration repository.

  @param [in]  This        Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS   Success
*/
STATIC
EFI_STATUS
EFIAPI
InitializePlatformRepository (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
  )
{
  EFI_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  PlatformRepo = This->PlatRepoInfo;

  GetJunoRevision (PlatformRepo->JunoRevision);
  DEBUG ((DEBUG_INFO, "Juno Rev = 0x%x\n", PlatformRepo->JunoRevision));
  return EFI_SUCCESS;
}

/** Return a standard namespace object.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       An optional token identifying the object. If
                           unused this must be CM_NULL_TOKEN.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
*/
EFI_STATUS
EFIAPI
GetStandardNameSpaceObject (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                             Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                   * CONST CmObject
  )
{
  EFI_STATUS                      Status = EFI_SUCCESS;
  EFI_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }
  PlatformRepo = This->PlatRepoInfo;

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    HANDLE_CM_OBJECT (EStdObjCfgMgrInfo, PlatformRepo->CmInfo);

    case EStdObjAcpiTableList:
      if (PlatformRepo->JunoRevision != JUNO_REVISION_R0) {
        CmObject->Size = sizeof (PlatformRepo->CmAcpiTableList);
      } else {
        UINT32 TableCount = sizeof (PlatformRepo->CmAcpiTableList) /
                              sizeof (PlatformRepo->CmAcpiTableList[0]);
        /* The last 2 tables in the ACPI table list enable PCIe support.
           Reduce the CmObject size so that the PCIe specific ACPI
           tables are not installed on Juno R0
        */
        CmObject->Size = sizeof (PlatformRepo->CmAcpiTableList[0]) *
                           (TableCount -2);
      }
      CmObject->Data = (VOID*)&PlatformRepo->CmAcpiTableList;
      DEBUG ((
        DEBUG_INFO,
        "EStdObjAcpiTableList: Ptr = 0x%p. Size = %d\n",
        CmObject->Data,
        CmObject->Size
        ));
      break;

    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** Return an ARM namespace object.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       An optional token identifying the object. If
                           unused this must be CM_NULL_TOKEN.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
*/
EFI_STATUS
EFIAPI
GetArmNameSpaceObject (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                             Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                   * CONST CmObject
  )
{
  EFI_STATUS                      Status = EFI_SUCCESS;
  EFI_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }
  PlatformRepo = This->PlatRepoInfo;

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    HANDLE_CM_OBJECT (EArmObjBootArchInfo, PlatformRepo->BootArchInfo);
    HANDLE_CM_OBJECT (
      EArmObjPowerManagementProfileInfo,
      PlatformRepo->PmProfileInfo
      );
    HANDLE_CM_OBJECT (EArmObjGenericTimerInfo, PlatformRepo->GenericTimerInfo);
    HANDLE_CM_OBJECT (
      EArmObjPlatformGenericWatchdogInfo,
      PlatformRepo->Watchdog
      );
    HANDLE_CM_OBJECT (EArmObjGicCInfo, PlatformRepo->GicCInfo);
    HANDLE_CM_OBJECT (EArmObjGicDInfo, PlatformRepo->GicDInfo);
    HANDLE_CM_OBJECT (
      EArmObjSerialConsolePortInfo,
      PlatformRepo->SpcrSerialPort
      );
    HANDLE_CM_OBJECT (EArmObjSerialDebugPortInfo, PlatformRepo->DbgSerialPort);
    HANDLE_CM_OBJECT (EArmObjGicMsiFrameInfo, PlatformRepo->GicMsiFrameInfo);

    case EArmObjPciConfigSpaceInfo:
      if (PlatformRepo->JunoRevision != JUNO_REVISION_R0) {
        CmObject->Size = sizeof (PlatformRepo->PciConfigInfo);
        CmObject->Data = (VOID*)&PlatformRepo->PciConfigInfo;
        DEBUG ((
          DEBUG_INFO,
          "EArmObjPciConfigSpaceInfo: Ptr = 0x%p, Size = %d\n",
          CmObject->Data,
          CmObject->Size
          ));
      }
      break;

    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_INFO,
        "INFO: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }//switch

  return Status;
}

/** Return an OEM namespace object.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       An optional token identifying the object. If
                           unused this must be CM_NULL_TOKEN.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
*/
EFI_STATUS
EFIAPI
GetOemNameSpaceObject (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                             Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                   * CONST CmObject
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The GetObject function defines the interface implemented by the
    Configuration Manager Protocol for returning the Configuration
    Manager Objects.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       An optional token identifying the object. If
                           unused this must be CM_NULL_TOKEN.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
*/
EFI_STATUS
EFIAPI
ArmJunoPlatformGetObject (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                             Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                   * CONST CmObject
  )
{
  EFI_STATUS  Status;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_NAMESPACE_ID (CmObjectId)) {
    case EObjNameSpaceStandard:
      Status = GetStandardNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceArm:
      Status = GetArmNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceOem:
      Status = GetOemNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    default: {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Unknown Namespace Object = 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The SetObject function defines the interface implemented by the
    Configuration Manager Protocol for updating the Configuration
    Manager Objects.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       An optional token identifying the object. If
                           unused this must be CM_NULL_TOKEN.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the Object.

  @retval EFI_UNSUPPORTED  This operation is not supported.
*/
EFI_STATUS
EFIAPI
ArmJunoPlatformSetObject (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                             Token OPTIONAL,
  IN        CM_OBJ_DESCRIPTOR                   * CONST CmObject
  )
{
  return EFI_UNSUPPORTED;
}

/** A structure describing the configuration manager protocol interface.
*/
STATIC
CONST
EFI_CONFIGURATION_MANAGER_PROTOCOL ArmJunoPlatformConfigManagerProtocol = {
  CREATE_REVISION (1, 0),
  ArmJunoPlatformGetObject,
  ArmJunoPlatformSetObject,
  &ArmJunoPlatformRepositoryInfo
};

/**
  Entrypoint of Configuration Manager Dxe.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
ConfigurationManagerDxeInitialize (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE  * SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiConfigurationManagerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID*)&ArmJunoPlatformConfigManagerProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to get Install Configuration Manager Protocol." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  Status = InitializePlatformRepository (
    &ArmJunoPlatformConfigManagerProtocol
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to initialize the Platform Configuration Repository." \
      " Status = %r\n",
      Status
      ));
  }

error_handler:
  return Status;
}