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
