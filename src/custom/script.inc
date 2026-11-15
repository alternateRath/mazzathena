// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

// ---- Mazzaro: Motor de Episodios, fase 2 (src/custom/mazzaro_episodio.hpp) ----
#include <custom/mazzaro_episodio.hpp>

static int32 buildin_mz_episodio_fora(mob_data* md, va_list ap)
{
	std::vector<int32>* ids = va_arg(ap, std::vector<int32>*);

	if (md->prev != nullptr && !mz_episodio::mob_ok(md->mob_id))
		ids->push_back(md->id);
	return 0;
}

/// mz_episodio <indice>{,"<tabela>"};
/// Define o lancamento atual (indice na cronologia; -1 desliga o filtro), relendo a tabela se vier o caminho, e tira
/// do mapa os monstros vivos que ficaram fora do episodio: spawn permanente sai e fica adiado ate o episodio chegar;
/// o resto (script, invocacao) e removido. Devolve quantos monstros sairam, ou -1 se a tabela nao abriu.
BUILDIN_FUNC(mz_episodio)
{
	if (script_hasdata(st, 3) && mz_episodio::carregar(script_getstr(st, 3)) < 0) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}
	mz_episodio::atual = script_getnum(st, 2);

	std::vector<int32> ids;
	map_foreachmob(buildin_mz_episodio_fora, &ids);
	for (int32 id : ids) {
		mob_data* md = map_id2md(id);

		if (md == nullptr)
			continue;
		if (md->spawn)
			mob_spawn(md); // sai do mapa; o gancho do mob_spawn adia a volta
		else
			unit_free(md, CLR_OUTSIGHT);
	}
	script_pushint(st, static_cast<int32>(ids.size()));
	return SCRIPT_CMD_SUCCESS;
}

static int32 buildin_mz_episodio_conta_sub(block_list* bl, va_list ap)
{
	int32 mob_id = va_arg(ap, int32);
	mob_data* md = BL_CAST(BL_MOB, bl);

	return (md != nullptr && md->mob_id == mob_id && !status_isdead(*md)) ? 1 : 0;
}

/// mz_episodio_conta(<id do monstro>, "<mapa>") -> quantos estao de fato no mapa e vivos (getmapunits conta tambem
/// os de spawn adiado, que existem como objeto fora do mapa). Usado pelo @episode conta e pelo teste.
BUILDIN_FUNC(mz_episodio_conta)
{
	int16 m = map_mapname2mapid(script_getstr(st, 3));

	if (m < 0) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}
	script_pushint(st, map_foreachinmap(buildin_mz_episodio_conta_sub, m, BL_MOB, script_getnum(st, 2)));
	return SCRIPT_CMD_SUCCESS;
}

/// mz_episodio_visivel("mob"|"item", <id>) -> 1 se o monstro/item existe no lancamento atual, 0 se nao.
BUILDIN_FUNC(mz_episodio_visivel)
{
	const char* tipo = script_getstr(st, 2);
	int32 id = script_getnum(st, 3);

	script_pushint(st, strcmp(tipo, "item") == 0 ? mz_episodio::item_ok(id) : mz_episodio::mob_ok(id));
	return SCRIPT_CMD_SUCCESS;
}

/// mobaro_ui <estado>{,<char id>};
/// MazzaRO: manda o estado do painel MobaRO ao cliente 2026 (ZC_UI_OPEN 0x0ae2 tipo OUT_UI_MOBARO = 0x4d, patch
/// mobaro-painel do mazzahex). O dword: bits 0-1 fase (0 fora, 1 fila, 2 partida), 2-9 fila, 10-17 total, 18-21 rota
/// (0-11; 15 = nenhuma), 22-24 e 25-27 placar, 28-29 vertente da rota (0-3; MobaRO v0.8, antes so o bit 28 da build),
/// 30 abre a janela, 31 fecha. Antes do estado vai o perfil (OUT_UI_MOBARO_PERFIL 0x50, bit 0 = sexo, 1 masculino): o
/// painel v2 escolhe por ele o retrato e Bard ou Dancer na rota 5. Clientes sem o patch ignoram os dois tipos.
BUILDIN_FUNC(mobaro_ui)
{
	map_session_data* sd;

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;
	clif_ui_open(*sd, OUT_UI_MOBARO_PERFIL, sd->status.sex == SEX_MALE ? 1 : 0);
	clif_ui_open(*sd, OUT_UI_MOBARO, script_getnum(st, 2));
	return SCRIPT_CMD_SUCCESS;
}

/// mobaro_hud <placar>,<relogio>{,<char id>};
/// MazzaRO: HUD da partida MobaRO no cliente 2026 (patch mobaro-hud do mazzahex), dois ZC_UI_OPEN 0x0ae2:
/// OUT_UI_MOBARO_HUD (0x4e) com o placar - bits 0-9 abates de jogador do time Kafra, 10-19 do time Rebenker, 20-29
/// abates do proprio jogador, 30 time dele (0 Kafra, 1 Rebenker), 31 esconde o HUD - e OUT_UI_MOBARO_RELOGIO (0x4f)
/// com o relogio - bits 0-15 segundos que faltam (o cliente conta sozinho), 31 desliga. Clientes sem o patch ignoram.
BUILDIN_FUNC(mobaro_hud)
{
	map_session_data* sd;

	if (!script_charid2sd(4, sd))
		return SCRIPT_CMD_FAILURE;
	clif_ui_open(*sd, OUT_UI_MOBARO_HUD, script_getnum(st, 2));
	clif_ui_open(*sd, OUT_UI_MOBARO_RELOGIO, script_getnum(st, 3));
	return SCRIPT_CMD_SUCCESS;
}

/// mobaro_modos <dword>{,<char id>};
/// MazzaRO: modos de jogo do painel MobaRO (janela "Escolha o modo", MobaRO v0.14): ZC_UI_OPEN 0x0ae2 tipo
/// OUT_UI_MOBARO_MODOS (0x51). O dword: bits 0-1 modo da ultima escolha (0 1x1, 1 3x3, 2 5x5), 2-3 / 4-5 / 6-7 faixa do
/// rank em cada modo (0 Bronze, 1 Prata, 2 Ouro), 8-11 / 12-15 / 16-19 quantos estao na fila de cada modo (0-15),
/// 20-29 pontos do rank no modo da ultima escolha (0-1023). Clientes sem o patch ignoram o tipo.
BUILDIN_FUNC(mobaro_modos)
{
	map_session_data* sd;

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;
	clif_ui_open(*sd, OUT_UI_MOBARO_MODOS, script_getnum(st, 2));
	return SCRIPT_CMD_SUCCESS;
}

/// mobaro_andar(<gid do monstro>,<x>,<y>) -> 1 se o caminho existe e o monstro saiu andando, 0 se nao.
/// MazzaRO: anda como o proprio monstro andaria (unit_walktoxy), SEM o "andar forcado" do unitwalk: o unitwalk liga
/// ud->state.force_walk, e com ele a IA do monstro nao procura alvo ate chegar a celula exata; se o caminho trava no
/// meio (outro monstro, jogador, parede), o monstro fica parado e cego para sempre (os lacaios do MobaRO ate a v0.13).
/// Aqui a IA continua valendo: inimigo a vista, ele larga o caminho e ataca; o script manda andar de novo depois.
/// Tambem solta um force_walk antigo que tenha ficado preso. Caminho de ate 32 celulas (MAX_WALKPATH).
BUILDIN_FUNC(mobaro_andar)
{
	block_list* bl = map_id2bl(script_getnum(st, 2));

	if (bl == nullptr || bl->type != BL_MOB || status_isdead(*bl)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	unit_data* ud = unit_bl2ud(bl);
	if (ud != nullptr)
		ud->state.force_walk = false;
	script_pushint(st, unit_walktoxy(bl, script_getnum(st, 3), script_getnum(st, 4), 0) ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

// ---- Mazzaro: moradores das cidades (npc/custom/mazzaro/moradores.txt, docs/bots.md) ----
// Personagens de verdade (linhas no banco, contas do grupo Morador) postos no mapa sem cliente pelo mesmo caminho do
// @autotrade restaurado no boot: pc_setnewpc com fd 0 (a sessao "vacuo" do map-server) + chrif_authreq autotrade.
// Ficam com autotrade == 1 e mz_morador; o pc_autotrade_timer chama OnMoradorEntrou em vez de abrir loja ou
// derrubar, e PM para eles vira OnMoradorPM (clif.cpp). Andar, falar, sentar e emote sao os unit* de sempre.

/// mz_morador_entrar <char id>;
/// Poe o morador no mapa (onde o personagem esta salvo). Devolve o account_id (o GID para unitwalk/unittalk) ou 0
/// se ja esta on, nao existe ou o autotrade esta desligado.
BUILDIN_FUNC(mz_morador_entrar)
{
	uint32 char_id = script_getnum(st, 2);

	script_pushint(st, 0);
	if (!battle_config.feature_autotrade || map_charid2sd(char_id) != nullptr)
		return SCRIPT_CMD_SUCCESS;
	if (SQL_ERROR == Sql_Query(mmysql_handle, "SELECT `account_id`, `sex` FROM `char` WHERE `char_id` = %u", char_id)) {
		Sql_ShowDebug(mmysql_handle);
		return SCRIPT_CMD_SUCCESS;
	}
	if (SQL_SUCCESS != Sql_NextRow(mmysql_handle)) {
		Sql_FreeResult(mmysql_handle);
		ShowWarning("mz_morador_entrar: personagem %u nao existe.\n", char_id);
		return SCRIPT_CMD_SUCCESS;
	}
	char* data;
	Sql_GetData(mmysql_handle, 0, &data, nullptr);
	uint32 account_id = static_cast<uint32>(strtoul(data, nullptr, 10));
	Sql_GetData(mmysql_handle, 1, &data, nullptr);
	char sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
	Sql_FreeResult(mmysql_handle);
	if (map_id2sd(account_id) != nullptr || chrif_search(account_id) != nullptr) // on, ou entrando (auth pendente)
		return SCRIPT_CMD_SUCCESS;

	map_session_data* sd;
	// como o login de cliente (clif.cpp): CREATE + placement new. O do_init_vending_autotrade do upstream so faz o
	// CREATE e o status_change fica sem construtor (pc_authok -> pc_setpos -> getSCE em ponteiro nulo).
	CREATE(sd, map_session_data, 1);
	new(sd) map_session_data();
	pc_setnewpc(sd, account_id, char_id, 0, gettick(), sex, 0);
	sd->state.autotrade = 1;
	sd->state.mz_morador = 1;
	sd->state.block_action |= PCBLOCK_IMMUNE;
	chrif_authreq(sd, true);
	script_pushint(st, account_id);
	return SCRIPT_CMD_SUCCESS;
}

/// mz_morador_sair <char id>;
/// Tira o morador do mapa (salva e desloga). Devolve 1 se havia um morador com esse char id.
BUILDIN_FUNC(mz_morador_sair)
{
	map_session_data* sd = map_charid2sd(script_getnum(st, 2));

	if (sd == nullptr || !sd->state.mz_morador) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	map_quit(sd);
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/// mz_morador_pm <char id do morador>,"<para>","<mensagem>";
/// O morador manda uma PM, como se tivesse digitado. Devolve 1 se o destinatario esta on.
BUILDIN_FUNC(mz_morador_pm)
{
	map_session_data* sd = map_charid2sd(script_getnum(st, 2));
	map_session_data* dst = map_nick2sd(script_getstr(st, 3), false);
	const char* msg = script_getstr(st, 4);

	if (sd == nullptr || !sd->state.mz_morador || dst == nullptr) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	clif_wis_message(dst, sd->status.name, msg, strlen(msg) + 1, 0);
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
