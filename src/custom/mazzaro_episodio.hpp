// Mazzaro: Motor de Episodios, fase 2 (P4 do projeto mazzaro, docs/episodios/MOTOR.md).
//
// Tabela id -> janela de lancamentos [abre, fecha) para monstros e itens, gerada pelo tools/episode_gen.py do
// mazzaro em db/import/mazzaro_episodios.txt, e o lancamento atual (indice na cronologia do bRO). O script
// npc/custom/mazzaro/episodios.txt chama `mz_episodio <indice>, "<tabela>";` no OnInit e a cada @episode.
// Um monstro ou item fora da tabela e sempre visivel; com o motor desligado (atual < 0) tudo e visivel.
//
// Quem consulta (procure "mz_episodio::" no codigo):
//   mob.cpp      mob_spawn (spawn permanente fica adiado), mob_once_spawn (script, @monster, invocacoes),
//                mob_get_random_id (Galho Seco e afins), drops normais e de MVP
//   npc.cpp      npc_buylist (compra recusada)       clif.cpp  clif_buylist (item some da lista da loja)
//   pc.cpp       pc_isUseitem, pc_isequip            atcommand.cpp  @item, @monster (avisam e recusam)
//   script.inc   mz_episodio, mz_episodio_visivel
#ifndef MAZZARO_EPISODIO_HPP
#define MAZZARO_EPISODIO_HPP

#include <cstdio>
#include <cstring>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/timer.hpp>

namespace mz_episodio {

// Visivel quando abre <= atual < fecha (fecha = lancamento que remove; sem remocao, um valor alto).
struct janela {
	int32 abre;
	int32 fecha;
};

inline std::unordered_map<int32, janela> mobs;
inline std::unordered_map<int32, janela> itens;
inline int32 atual = -1;

// Intervalo para tentar de novo um spawn permanente de monstro fora do episodio.
constexpr t_tick RETENTAR_MS = 10000;

inline bool visivel( const std::unordered_map<int32, janela>& tabela, int32 id ){
	if( atual < 0 ){
		return true;
	}
	auto it = tabela.find( id );
	if( it == tabela.end() ){
		return true;
	}
	return it->second.abre <= atual && atual < it->second.fecha;
}

inline bool mob_ok( int32 mob_id ){
	return visivel( mobs, mob_id );
}

inline bool item_ok( t_itemid nameid ){
	return visivel( itens, static_cast<int32>( nameid ) );
}

// Le a tabela: linhas "M,<id>,<abre>,<fecha>" (monstro) e "I,<id>,<abre>,<fecha>" (item); '//' comenta.
// Devolve quantas entradas leu, ou -1 se o arquivo nao abriu (a tabela anterior fica).
inline int32 carregar( const char* caminho ){
	FILE* fp = fopen( caminho, "r" );

	if( fp == nullptr ){
		ShowError( "mz_episodio: nao abriu '%s'.\n", caminho );
		return -1;
	}

	std::unordered_map<int32, janela> m, i;
	char linha[1024];
	int32 n = 0, nlinha = 0;

	while( fgets( linha, sizeof( linha ), fp ) != nullptr ){
		char tipo;
		int32 id, abre, fecha;

		nlinha++;
		if( strchr( linha, '\n' ) == nullptr && !feof( fp ) ){ // linha maior que o buffer: descarta o resto
			int c;
			while( ( c = fgetc( fp ) ) != EOF && c != '\n' );
		}
		if( linha[0] == '/' || linha[0] == '\r' || linha[0] == '\n' || linha[0] == '\0' ){
			continue;
		}
		if( sscanf( linha, "%c,%d,%d,%d", &tipo, &id, &abre, &fecha ) != 4 || ( tipo != 'M' && tipo != 'I' ) ){
			ShowWarning( "mz_episodio: linha %d invalida em '%s'.\n", nlinha, caminho );
			continue;
		}
		( tipo == 'M' ? m : i )[id] = janela{ abre, fecha };
		n++;
	}
	fclose( fp );
	mobs.swap( m );
	itens.swap( i );
	ShowStatus( "mz_episodio: %d monstros e %d itens com janela de episodio ('%s').\n", (int32)mobs.size(), (int32)itens.size(), caminho );
	return n;
}

} // namespace mz_episodio

#endif /* MAZZARO_EPISODIO_HPP */
