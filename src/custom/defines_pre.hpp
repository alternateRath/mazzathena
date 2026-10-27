// Mazzaro: defines aplicados antes de qualquer config do rAthena (copiado para ../rathena/src/custom/).
#ifndef CONFIG_CUSTOM_DEFINES_PRE_HPP
#define CONFIG_CUSTOM_DEFINES_PRE_HPP

// Servidor pre-renewal: formulas, drops e exp classicos.
#define PRERE

// PACKETVER do perfil (server/profiles/<perfil>.yml), gravado pelo build.ps1 -Perfil neste arquivo fora do git:
// cada pasta de build (rathena = kro-2020, rathena-2026 = kro-2026) tem o seu.
#if __has_include("mazzaro_packetver.hpp")
	#include "mazzaro_packetver.hpp"
#endif

// Sem o arquivo: cliente kRO 2020-03-04 (Ragexe.exe extraido do RAG_SETUP_200309, timestamp PE 2020-03-02).
#ifndef PACKETVER
	#define PACKETVER 20200304
#endif

#endif /* CONFIG_CUSTOM_DEFINES_PRE_HPP */
