// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),
BUILDIN_DEF(mz_episodio,"i?"), // Mazzaro: Motor de Episodios
BUILDIN_DEF(mz_episodio_visivel,"si"),
BUILDIN_DEF(mz_episodio_conta,"is"),
BUILDIN_DEF(mobaro_ui,"i?"), // Mazzaro: painel MobaRO do cliente 2026
BUILDIN_DEF(mobaro_hud,"ii?"), // Mazzaro: HUD da partida MobaRO (placar no topo, abates, relogio)
BUILDIN_DEF(mobaro_modos,"i??"), // Mazzaro: modos de jogo do painel MobaRO (rank e fila de 1x1, 3x3, 5x5)
BUILDIN_DEF(mobaro_andar,"iii"), // Mazzaro: monstro anda sem o force_walk do unitwalk (lacaios do MobaRO)
BUILDIN_DEF(mz_morador_entrar,"i"), // Mazzaro: moradores das cidades (sessao sem cliente)
BUILDIN_DEF(mz_morador_sair,"i"),
BUILDIN_DEF(mz_morador_pm,"iss"),
