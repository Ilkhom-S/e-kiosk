/* @file Названия протоколов устройств. */

#pragma once

//--------------------------------------------------------------------------------
namespace ProtocolNames {
    namespace CashDevice {
        const char CCTalk[] = "ccTalk";
        const char CCNet[] = "CCNet";
        const char SSP[] = "SSP";
    } // namespace CashDevice

    namespace CashAcceptor {
        const char NPSTalk[] = "NPSTalk";
        const char ID003[] = "ID003";
        const char EBDS[] = "EBDS";
        const char V2e[] = "V2e";
        const char ICT[] = "ICT";
    } // namespace CashAcceptor

    namespace FR {
        const char ATOL2[] = "ATOL2";
        const char ATOL3[] = "ATOL3";
        const char Shtrih[] = "Shtrih";
        const char PRIM[] = "PRIM";
        const char SPARK[] = "SPARK";
        const char Kasbi[] = "Kasbi";
        const char AFP[] = "AFP";
        const char Incotex[] = "Incotex";
    } // namespace FR

    namespace Dispenser {
        const char Puloon[] = "Puloon";
    } // namespace Dispenser

    namespace Cardreader {
        const char Creator[] = "Creator";
    } // namespace Cardreader
} // namespace ProtocolNames
//--------------------------------------------------------------------------------
