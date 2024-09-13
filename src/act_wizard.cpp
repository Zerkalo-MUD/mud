/* ************************************************************************
*   File: act.wizard.cpp                                Part of Bylins    *
*  Usage: Player-level god commands and other goodies                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author$                                                       *
*  $Date$                                           *
*  $Revision$                                                      *
************************************************************************ */

#include "act_wizard.h"

#include "action_targeting.h"
#include "cmd_god/ban.h"
#include "game_mechanics/birthplaces.h"
#include "game_mechanics/celebrates.h"
#include "game_mechanics/dead_load.h"
#include "game_mechanics/guilds.h"
#include "utils/utils_char_obj.inl"
#include "entities/char_data.h"
#include "entities/obj_data.h"
#include "entities/char_player.h"
#include "game_mechanics/player_races.h"
#include "entities/entities_constants.h"
#include "entities/world_characters.h"
#include "cmd_god/stat.h"
#include "cmd/follow.h"
#include "comm.h"
#include "cmd_god/shutdown.h"
#include "conf.h"
#include "config.h"
#include "constants.h"
#include "corpse.h"
#include "db.h"
#include "depot.h"
#include "description.h"
#include "dg_script/dg_scripts.h"
#include "dg_script/dg_event.h"
#include "game_economics/ext_money.h"
#include "game_fight/fight.h"
#include "game_fight/pk.h"
#include "utils/file_crc.h"
#include "genchar.h"
#include "structs/global_objects.h"
#include "game_classes/classes.h"
#include "game_mechanics/glory.h"
#include "game_mechanics/glory_const.h"
#include "game_mechanics/glory_misc.h"
#include "game_mechanics/mem_queue.h"
#include "handler.h"
#include "heartbeat.h"
#include "house.h"
#include "game_crafts/im.h"
#include "interpreter.h"
#include "liquid.h"
#include "utils/logger.h"
#include "communication/mail.h"
#include "statistics/mob_stat.h"
#include "modify.h"
#include "administration/names.h"
#include "entities/obj_data.h"
#include "obj_prototypes.h"
#include "olc/olc.h"
#include "communication/parcel.h"
#include "administration/password.h"
#include "administration/privilege.h"
#include "third_party_libs/pugixml/pugixml.h"
#include "entities/room_data.h"
#include "color.h"
#include "game_mechanics/sets_drop.h"
#include "game_skills/skills.h"
#include "game_magic/spells.h"
#include "structs/descriptor_data.h"
#include "structs/structs.h"
#include "sysdep.h"
#include "title.h"
#include "statistics/top.h"
#include "utils/utils.h"
#include "utils/id_converter.h"
#include "entities/world_objects.h"
#include "entities/zone.h"
#include "game_classes/classes_constants.h"
#include "game_magic/spells_info.h"
#include "game_magic/magic_rooms.h"
#include "olc/olc.h"
#include <third_party_libs/fmt/include/fmt/format.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::fstream;

// external vars
extern FILE *player_fl;
extern int circle_restrict;
extern int load_into_inventory;
extern time_t zones_stat_date;
extern void DecayObjectsOnRepop(std::vector<ZoneRnum> &zone_list);    // рассыпание обьектов ITEM_REPOP_DECAY
extern int check_dupes_host(DescriptorData *d, bool autocheck = false);

void medit_save_to_disk(int zone_num);

// for entities
void do_recall(CharData *ch, char *argument, int cmd, int subcmd);
void log_zone_count_reset();
// extern functions
void appear(CharData *ch);
void ResetZone(ZoneRnum zone);
//extern CharData *find_char(long n);
int _parse_name(char *arg, char *name);
int Valid_Name(char *name);
int reserved_word(const char *name);
// local functions
void perform_immort_invis(CharData *ch, int level);
void do_echo(CharData *ch, char *argument, int cmd, int subcmd);
void do_send(CharData *ch, char *argument, int cmd, int subcmd);
RoomRnum find_target_room(CharData *ch, char *rawroomstr, int trig);
void do_at(CharData *ch, char *argument, int cmd, int subcmd);
void do_goto(CharData *ch, char *argument, int cmd, int subcmd);
void do_teleport(CharData *ch, char *argument, int cmd, int subcmd);
void do_shutdown(CharData *ch, char *argument, int cmd, int subcmd);
void stop_snooping(CharData *ch);
void do_snoop(CharData *ch, char *argument, int cmd, int subcmd);
void do_switch(CharData *ch, char *argument, int cmd, int subcmd);
void do_return(CharData *ch, char *argument, int cmd, int subcmd);
void do_load(CharData *ch, char *argument, int cmd, int subcmd);
void do_vstat(CharData *ch, char *argument, int cmd, int subcmd);
void do_purge(CharData *ch, char *argument, int cmd, int subcmd);
void do_syslog(CharData *ch, char *argument, int cmd, int subcmd);
void do_advance(CharData *ch, char *argument, int cmd, int subcmd);
void do_restore(CharData *ch, char *argument, int cmd, int subcmd);
void perform_immort_vis(CharData *ch);
void do_invis(CharData *ch, char *argument, int cmd, int subcmd);
void do_gecho(CharData *ch, char *argument, int cmd, int subcmd);
void do_poofset(CharData *ch, char *argument, int cmd, int subcmd);
void do_dc(CharData *ch, char *argument, int cmd, int subcmd);
void do_wizlock(CharData *ch, char *argument, int cmd, int subcmd);
void do_date(CharData *ch, char *argument, int cmd, int subcmd);
void do_last(CharData *ch, char *argument, int cmd, int subcmd);
void do_force(CharData *ch, char *argument, int cmd, int subcmd);
void do_wiznet(CharData *ch, char *argument, int cmd, int subcmd);
void do_zreset(CharData *ch, char *argument, int cmd, int subcmd);
void do_wizutil(CharData *ch, char *argument, int cmd, int subcmd);
void do_show(CharData *ch, char *argument, int cmd, int subcmd);
void do_liblist(CharData *ch, char *argument, int cmd, int subcmd);
//
void do_sdemigod(CharData *ch, char *argument, int cmd, int subcmd);
void do_unfreeze(CharData *ch, char *argument, int cmd, int subcmd);
void do_setall(CharData *ch, char *argument, int cmd, int subcmd);
void do_check_occupation(CharData *ch, char *argument, int cmd, int subcmd);
void do_delete_obj(CharData *ch, char *argument, int cmd, int subcmd);
void do_arena_restore(CharData *ch, char *argument, int cmd, int subcmd);
void do_showzonestats(CharData *, char *, int, int);
void do_overstuff(CharData *ch, char *, int, int);
void do_send_text_to_char(CharData *ch, char *, int, int);
void generate_magic_enchant(ObjData *obj);

void log_zone_count_reset() {
	for (auto & i : zone_table) {
		sprintf(buf, "Zone: %d, count_reset: %d", i.vnum, i.count_reset);
		log("%s", buf);
	}
}

// Отправляет любой текст выбранному чару
void do_send_text_to_char(CharData *ch, char *argument, int, int) {
	CharData *vict = nullptr;

	half_chop(argument, buf, buf2);

	if (!*buf || !*buf2) {
		SendMsgToChar("Кому и какой текст вы хотите отправить?\r\n", ch);
	} else if (!(vict = get_player_vis(ch, buf, EFind::kCharInWorld))) {
		SendMsgToChar("Такого персонажа нет в игре.\r\n", ch);
	} else if (vict->IsNpc())
		SendMsgToChar("Такого персонажа нет в игре.\r\n", ch);
	else {
		snprintf(buf1, kMaxStringLength, "%s\r\n", buf2);
		SendMsgToChar(buf1, vict);
	}
}

// показывает количество вещей (чтобы носить которые, нужно больше 8 ремортов) в хранах кланов
void do_overstuff(CharData *ch, char *, int, int) {
	std::map<std::string, int> objects;
	for (const auto & clan : Clan::ClanList) {
		for (ObjData *chest = world[GetRoomRnum(clan->get_chest_room())]->contents; chest;
			 chest = chest->get_next_content()) {
			if (Clan::is_clan_chest(chest)) {
				for (ObjData *temp = chest->get_contains(); temp; temp = temp->get_next_content()) {
					if (temp->get_auto_mort_req() > 8) {
						if (objects.count(clan->get_abbrev())) {
							objects[clan->get_abbrev()] += 1;
						} else {
							objects.insert(std::pair<std::string, int>(clan->get_abbrev(), 1));
						}
					}
				}
			}
		}
	}

	for (auto & object : objects) {
		sprintf(buf, "Дружина: %s, количество объектов: %d\r\n", object.first.c_str(), object.second);
		SendMsgToChar(buf, ch);
	}
}

// Функция для отправки текста богам
// При demigod = True, текст отправляется и демигодам тоже
void send_to_gods(char *text, bool demigod) {
	DescriptorData *d;
	for (d = descriptor_list; d; d = d->next) {
		// Чар должен быть в игре
		if (STATE(d) == CON_PLAYING) {
			// Чар должен быть имморталом
			// Либо же демигодом (при demigod = true)
			if ((GetRealLevel(d->character) >= kLvlGod) ||
				(GET_GOD_FLAG(d->character, EGf::kDemigod) && demigod)) {
				SendMsgToChar(text, d->character.get());
			}
		}
	}
}

extern const char *deaf_social;

// Adds karma string to KARMA
// \TODO Move to PlayerData
void AddKarma(CharData *ch, const char *punish, const char *reason) {
	if (reason && (reason[0] != '.')) {
		char smallbuf[kMaxInputLength];
		time_t nt = time(nullptr);
		snprintf(smallbuf, kMaxInputLength, "%s :: %s [%s]\r\n", rustime(localtime(&nt)), punish, reason);
		KARMA(ch) = str_add(KARMA(ch), smallbuf);
	}
}

void do_delete_obj(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int vnum;
	one_argument(argument, buf);
	int num = 0;
	if (!*buf || !a_isdigit(*buf)) {
		SendMsgToChar("Usage: delete <number>\r\n", ch);
		return;
	}
	if ((vnum = atoi(buf)) < 0) {
		SendMsgToChar("Указан неверный VNUM объекта !\r\n", ch);
		return;
	}

	world_objects.foreach_with_vnum(vnum, [&num](const ObjData::shared_ptr &k) {
		k->set_timer(0);
		++num;
	});
//	кланхран вещи в игре их не надо чистить, удалены выше.
//	num += Clan::delete_obj(vnum);
	num += Depot::delete_obj(vnum);
	num += Parcel::delete_obj(vnum);
	sprintf(buf2, "Удалено всего предметов: %d, смотрим ренту.\r\n", num);
	SendMsgToChar(buf2, ch);
	num = 0;
	for (std::size_t pt_num = 0; pt_num< player_table.size(); pt_num++) {
		bool need_save = false;
	// рента
		if (player_table[pt_num].timer) {
			for (auto i = player_table[pt_num].timer->time.begin(),
					 iend = player_table[pt_num].timer->time.end(); i != iend; ++i) {
				if (i->vnum == vnum && i->timer > 0) {
					num++;
					sprintf(buf2, "Player %s : item [%d] deleted\r\n", player_table[pt_num].name(), i->vnum);;
					SendMsgToChar(buf2, ch);
					i->timer = -1;
					int rnum = GetObjRnum(i->vnum);
					if (rnum >= 0) {
						obj_proto.dec_stored(rnum);
					}
					need_save = true;
				}
			}
		}
		if (need_save) {
			if (!Crash_write_timer(pt_num)) {
				sprintf(buf, "SYSERROR: [TO] Error writing timer file for %s", player_table[pt_num].name());
				SendMsgToChar(buf2, ch);
			}
		}
	}
	sprintf(buf2, "Удалено еще предметов: %d.\r\n", num);
	SendMsgToChar(buf2, ch);
}

bool comp(std::pair <int, int> a, std::pair<int, int> b) {
	return a.second > b.second;
}

void PrintZoneStat(CharData *ch, int start, int end, bool sort) {
	std::stringstream ss;

	if (end == 0)
		end = start;
	std::vector<std::pair<int, int>> zone;
	for (ZoneRnum i = start; i < static_cast<ZoneRnum>(zone_table.size()) && i <= end; i++) {
		zone.push_back(std::make_pair(i, zone_table[i].traffic));
	}
	if (sort) {
		std::sort(zone.begin(), zone.end(), comp);
	}
	for (auto it : zone) {
		ss << "Zone: " << zone_table[it.first].vnum << " count_reset с ребута: " << zone_table[it.first].count_reset 
					<< ", посещено: " << zone_table[it.first].traffic << ", название зоны: " << zone_table[it.first].name<< "\r\n";
	}
	page_string(ch->desc, ss.str());
}

void do_showzonestats(CharData *ch, char *argument, int, int) {
	std::string buffer;
	char arg1[kMaxInputLength], arg2[kMaxInputLength], arg3[kMaxInputLength];
	bool sort = false;

	three_arguments(argument, arg1, arg2, arg3);
	if (!str_cmp(arg2, "-s") || !str_cmp(arg3, "-s"))
		sort = true;
	if (!*arg1) {
		SendMsgToChar(ch, "Зоныстат формат: 'все' или диапазон через пробел, -s в конце для сортировки. 'очистить' новая таблица\r\n");
		return;
	}
	if (!str_cmp(arg1, "очистить")) {
		const time_t ct = time(nullptr);
		char *date = asctime(localtime(&ct));
		SendMsgToChar(ch, "Начинаю новую запись статистики от %s", date);
		zones_stat_date = ct;
		for (auto & i : zone_table) {
			i.traffic = 0;
		}
		ZoneTrafficSave();
		return;
	}
	SendMsgToChar(ch, "Статистика с %s", asctime(localtime(&zones_stat_date)));
	if (!str_cmp(arg1, "все")) {
		PrintZoneStat(ch, 0, 999999, sort);
		return;
	}
	int tmp1 = GetZoneRnum(atoi(arg1));
	int tmp2 = GetZoneRnum(atoi(arg2));
	if (tmp1 >= 0 && !*arg2) {
		PrintZoneStat(ch, tmp1, tmp1, sort);
		return;
	}
	if (tmp1 > 0 && tmp2 > 0) {
		PrintZoneStat(ch, tmp1, tmp2, sort);
		return;
	}
	SendMsgToChar(ch, "Зоныстат формат: 'все' или диапазон через пробел, -s в конце для сортировки. 'очистить' новая таблица\r\n");
}

void do_arena_restore(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *vict;

	one_argument(argument, buf);
	if (!*buf)
		SendMsgToChar("Кого вы хотите восстановить?\r\n", ch);
	else if (!(vict = get_char_vis(ch, buf, EFind::kCharInWorld)))
		SendMsgToChar(NOPERSON, ch);
	else {
		GET_HIT(vict) = GET_REAL_MAX_HIT(vict);
		GET_MOVE(vict) = GET_REAL_MAX_MOVE(vict);
		if (IS_MANA_CASTER(vict)) {
			vict->mem_queue.stored = GET_MAX_MANA(vict);
		} else {
			vict->mem_queue.stored = vict->mem_queue.total;
		}
		if (vict->GetSkill(ESkill::kWarcry) > 0) {
			struct TimedSkill wctimed;
			wctimed.skill = ESkill::kWarcry;
			wctimed.time = 0;
			ImposeTimedSkill(vict, &wctimed);
		}
		if (IS_GRGOD(ch) && IS_IMMORTAL(vict)) {
			vict->set_str(25);
			vict->set_int(25);
			vict->set_wis(25);
			vict->set_dex(25);
			vict->set_con(25);
			vict->set_cha(25);
		}
		update_pos(vict);
		RemoveAffectFromChar(vict, ESpell::kDrunked);
		GET_DRUNK_STATE(vict) = GET_COND(vict, DRUNK) = 0;
		RemoveAffectFromChar(vict, ESpell::kAbstinent);

		//сброс таймеров скиллов и фитов
		while (vict->timed)
			ExpireTimedSkill(vict, vict->timed);
		while (vict->timed_feat)
			ExpireTimedFeat(vict, vict->timed_feat);
		reset_affects(vict);
		for (int i = 0; i < EEquipPos::kNumEquipPos; i++) {
			if (GET_EQ(vict, i)) {
				remove_otrigger(GET_EQ(vict, i), vict);
				ExtractObjFromWorld(UnequipChar(vict, i, CharEquipFlags()));
			}
		}
		ObjData *obj;
		for (obj = vict->carrying; obj; obj = vict->carrying) {
			RemoveObjFromChar(obj);
			ExtractObjFromWorld(obj);
		}
		act("Все ваши вещи были удалены и все аффекты сняты $N4!",
			false, vict, nullptr, ch, kToChar);
	}
}

int set_punish(CharData *ch, CharData *vict, int punish, char *reason, long times) {
	Punish *pundata = nullptr;
	int result;

	if (ch == vict) {
		SendMsgToChar("Это слишком жестоко...\r\n", ch);
		return 0;
	}
	if ((GetRealLevel(vict) >= kLvlImmortal && !IS_IMPL(ch)) || IS_IMPL(vict)) {
		SendMsgToChar("Кем вы себя возомнили?\r\n", ch);
		return 0;
	}
	// Проверяем а может ли чар вообще работать с этим наказанием.
	switch (punish) {
		case SCMD_MUTE: pundata = &(vict)->player_specials->pmute;
			break;
		case SCMD_DUMB: pundata = &(vict)->player_specials->pdumb;
			break;
		case SCMD_HELL: pundata = &(vict)->player_specials->phell;
			break;
		case SCMD_NAME: pundata = &(vict)->player_specials->pname;
			break;
		case SCMD_FREEZE: pundata = &(vict)->player_specials->pfreeze;
			break;
		case SCMD_REGISTER:
		case SCMD_UNREGISTER: pundata = &(vict)->player_specials->punreg;
			break;
	}
	assert(pundata);
	if (GetRealLevel(ch) < pundata->level) {
		SendMsgToChar("Да кто ты такой?!! Чтобы оспаривать волю СТАРШИХ БОГОВ!!!\r\n", ch);
		return 0;
	}
	// Проверяем наказание или освобождение.
	if (times == 0) {
		// Чара досрочно освобождают от наказания.
		if (!reason || !*reason) {
			SendMsgToChar("Укажите причину такой милости.\r\n", ch);
			return 0;
		} else
			skip_spaces(&reason);
		//
		pundata->duration = 0;
		pundata->level = 0;
		pundata->godid = 0;
		if (pundata->reason)
			free(pundata->reason);
		pundata->reason = nullptr;
		switch (punish) {
			case SCMD_MUTE:
				if (!vict->IsFlagged(EPlrFlag::kMuted)) {
					SendMsgToChar("Ваша жертва и так может кричать.\r\n", ch);
					return (0);
				};
				vict->UnsetFlag(EPlrFlag::kMuted);
				sprintf(buf, "Mute OFF for %s by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Mute OFF by %s", GET_NAME(ch));
				AddKarma(vict, buf, reason);
				sprintf(buf, "%s%s разрешил$G вам кричать.%s", CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n2 вернулся голос.");
				break;
			case SCMD_FREEZE:
				if (!vict->IsFlagged(EPlrFlag::kFrozen)) {
					SendMsgToChar("Ваша жертва уже разморожена.\r\n", ch);
					return (0);
				};
				vict->UnsetFlag(EPlrFlag::kFrozen);
				Glory::remove_freeze(GET_UNIQUE(vict));
				if (vict->IsFlagged(EPlrFlag::kHelled)) //заодно снимем ад
					vict->UnsetFlag(EPlrFlag::kHelled);
				sprintf(buf, "Freeze OFF for %s by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Freeze OFF by %s", GET_NAME(ch));
				AddKarma(vict, buf, reason);
				if (vict->in_room != kNowhere) {
					act("$n выпущен$a из темницы!", false, vict, nullptr, nullptr, kToRoom);
					if ((result = GET_LOADROOM(vict)) == kNowhere)
						result = calc_loadroom(vict);
					result = GetRoomRnum(result);
					if (result == kNowhere) {
						if (GetRealLevel(vict) >= kLvlImmortal)
							result = r_immort_start_room;
						else
							result = r_mortal_start_room;
					}
					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, result);
					look_at_room(vict, result);
				};
				sprintf(buf, "%s%s выпустил$G вас из темницы.%s",
						CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n выпущен$a из темницы!");
				sprintf(buf, "%sЛедяные оковы растаяли под добрым взглядом $N1.%s",
						CCIYEL(vict, C_NRM), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n освободил$u из ледяного плена.");
				break;
			case SCMD_DUMB:
				if (!vict->IsFlagged(EPlrFlag::kDumbed)) {
					SendMsgToChar("Ваша жертва и так может издавать звуки.\r\n", ch);
					return (0);
				};
				vict->UnsetFlag(EPlrFlag::kDumbed);

				sprintf(buf, "Dumb OFF for %s by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Dumb OFF by %s", GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s разрешил$G вам издавать звуки.%s",
						CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));

				sprintf(buf2, "$n нарушил$g обет молчания.");

				break;

			case SCMD_HELL:
				if (!vict->IsFlagged(EPlrFlag::kHelled)) {
					SendMsgToChar("Ваша жертва и так на свободе.\r\n", ch);
					return (0);
				};
				vict->UnsetFlag(EPlrFlag::kHelled);

				sprintf(buf, "%s removed FROM hell by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Removed FROM hell by %s", GET_NAME(ch));
				AddKarma(vict, buf, reason);

				if (vict->in_room != kNowhere) {
					act("$n выпущен$a из темницы!",
						false, vict, nullptr, nullptr, kToRoom);

					if ((result = GET_LOADROOM(vict)) == kNowhere)
						result = calc_loadroom(vict);

					result = GetRoomRnum(result);

					if (result == kNowhere) {
						if (GetRealLevel(vict) >= kLvlImmortal)
							result = r_immort_start_room;
						else
							result = r_mortal_start_room;
					}
					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, result);
					look_at_room(vict, result);
				};

				sprintf(buf, "%s%s выпустил$G вас из темницы.%s",
						CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));

				sprintf(buf2, "$n выпущен$a из темницы!");
				break;

			case SCMD_NAME:

				if (!vict->IsFlagged(EPlrFlag::kNameDenied)) {
					SendMsgToChar("Вашей жертвы там нет.\r\n", ch);
					return (0);
				};
				vict->UnsetFlag(EPlrFlag::kNameDenied);

				sprintf(buf, "%s removed FROM name room by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Removed FROM name room by %s", GET_NAME(ch));
				AddKarma(vict, buf, reason);

				if (vict->in_room != kNowhere) {
					if ((result = GET_LOADROOM(vict)) == kNowhere)
						result = calc_loadroom(vict);

					result = GetRoomRnum(result);

					if (result == kNowhere) {
						if (GetRealLevel(vict) >= kLvlImmortal)
							result = r_immort_start_room;
						else
							result = r_mortal_start_room;
					}

					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, result);
					look_at_room(vict, result);
					act("$n выпущен$a из комнаты имени!",
						false, vict, nullptr, nullptr, kToRoom);
				};
				sprintf(buf, "%s%s выпустил$G вас из комнаты имени.%s",
						CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));

				sprintf(buf2, "$n выпущен$a из комнаты имени!");
				break;

			case SCMD_REGISTER:
				// Регистриуем чара
				if (vict->IsFlagged(EPlrFlag::kRegistred)) {
					SendMsgToChar("Вашей жертва уже зарегистрирована.\r\n", ch);
					return (0);
				};

				sprintf(buf, "%s registered by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Registered by %s", GET_NAME(ch));
				RegisterSystem::add(vict, buf, reason);
				AddKarma(vict, buf, reason);

				if (vict->in_room != kNowhere) {

					act("$n зарегистрирован$a!", false, vict,
						nullptr, nullptr, kToRoom);

					if ((result = GET_LOADROOM(vict)) == kNowhere)
						result = calc_loadroom(vict);

					result = GetRoomRnum(result);

					if (result == kNowhere) {
						if (GetRealLevel(vict) >= kLvlImmortal)
							result = r_immort_start_room;
						else
							result = r_mortal_start_room;
					}

					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, result);
					look_at_room(vict, result);
				};
				sprintf(buf, "%s%s зарегистрировал$G вас.%s",
						CCIGRN(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n появил$u в центре комнаты, с гордостью показывая всем штампик регистрации!");
				break;
			case SCMD_UNREGISTER:
				if (!vict->IsFlagged(EPlrFlag::kRegistred)) {
					SendMsgToChar("Ваша цель и так не зарегистрирована.\r\n", ch);
					return (0);
				};

				sprintf(buf, "%s unregistered by %s.", GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Unregistered by %s", GET_NAME(ch));
				RegisterSystem::remove(vict);
				AddKarma(vict, buf, reason);

				if (vict->in_room != kNowhere) {
					act("C $n1 снята метка регистрации!",
						false, vict, nullptr, nullptr, kToRoom);
					/*				if ((result = GET_LOADROOM(vict)) == kNowhere)
									result = r_unreg_start_room;

								result = GetRoomRnum(result);

								if (result == kNowhere)
								{
									if (GetRealLevel(vict) >= kLevelImmortal)
										result = r_immort_start_room;
									else
										result = r_mortal_start_room;
								}

								char_from_room(vict);
								char_to_room(vict, result);
								look_at_room(vict, result);
				*/
				}
				sprintf(buf, "&W%s снял$G с вас метку регистрации.&n", GET_NAME(ch));
				sprintf(buf2, "$n лишен$g регистрации!");
				break;
		}
	} else {
		// Чара наказывают.
		if (!reason || !*reason) {
			SendMsgToChar("Укажите причину наказания.\r\n", ch);
			return 0;
		} else
			skip_spaces(&reason);

		pundata->level = ch->IsFlagged(EPrf::kCoderinfo) ? kLvlImplementator : GetRealLevel(ch);
		pundata->godid = GET_UNIQUE(ch);

		// Добавляем в причину имя имма

		sprintf(buf, "%s : %s", GET_NAME(ch), reason);
		pundata->reason = str_dup(buf);

		switch (punish) {
			case SCMD_MUTE: vict->SetFlag(EPlrFlag::kMuted);
				pundata->duration = (times > 0) ? time(nullptr) + times * 60 * 60 : MAX_TIME;

				sprintf(buf, "Mute ON for %s by %s(%ldh).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Mute ON (%ldh) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s запретил$G вам кричать.%s",
						CCIRED(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));

				sprintf(buf2, "$n подавился своим криком.");

				break;

			case SCMD_FREEZE: vict->SetFlag(EPlrFlag::kFrozen);
				Glory::set_freeze(GET_UNIQUE(vict));
				pundata->duration = (times > 0) ? time(nullptr) + times * 60 * 60 : MAX_TIME;

				sprintf(buf, "Freeze ON for %s by %s(%ldh).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Freeze ON (%ldh) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%sАдский холод сковал ваше тело ледяным панцирем.\r\n%s",
						CCIBLU(vict, C_NRM), CCNRM(vict, C_NRM));
				sprintf(buf2, "Ледяной панцирь покрыл тело $n1! Стало очень тихо и холодно.");
				if (vict->in_room != kNowhere) {
					act("$n водворен$a в темницу!",
						false, vict, nullptr, nullptr, kToRoom);

					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, r_helled_start_room);
					look_at_room(vict, r_helled_start_room);
				};
				break;

			case SCMD_DUMB: vict->SetFlag(EPlrFlag::kDumbed);
				pundata->duration = (times > 0) ? time(nullptr) + times * 60 : MAX_TIME;

				sprintf(buf, "Dumb ON for %s by %s(%ldm).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);

				sprintf(buf, "Dumb ON (%ldm) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s запретил$G вам издавать звуки.%s",
						CCIRED(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));

				sprintf(buf2, "$n дал$g обет молчания.");
				break;
			case SCMD_HELL: vict->SetFlag(EPlrFlag::kHelled);

				pundata->duration = (times > 0) ? time(nullptr) + times * 60 * 60 : MAX_TIME;

				if (vict->in_room != kNowhere) {
					act("$n водворен$a в темницу!",
						false, vict, nullptr, nullptr, kToRoom);

					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, r_helled_start_room);
					look_at_room(vict, r_helled_start_room);
				};
				vict->set_was_in_room(kNowhere);

				sprintf(buf, "%s moved TO hell by %s(%ldh).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Moved TO hell (%ldh) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s поместил$G вас в темницу.%s", GET_NAME(ch),
						CCIRED(vict, C_NRM), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n водворен$a в темницу!");
				break;

			case SCMD_NAME: vict->SetFlag(EPlrFlag::kNameDenied);

				pundata->duration = (times > 0) ? time(nullptr) + times * 60 * 60 : MAX_TIME;

				if (vict->in_room != kNowhere) {
					act("$n водворен$a в комнату имени!",
						false, vict, nullptr, nullptr, kToRoom);
					RemoveCharFromRoom(vict);
					PlaceCharToRoom(vict, r_named_start_room);
					look_at_room(vict, r_named_start_room);
				};
				vict->set_was_in_room(kNowhere);

				sprintf(buf, "%s removed to nameroom by %s(%ldh).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Removed TO nameroom (%ldh) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s поместил$G вас в комнату имени.%s",
						CCIRED(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n помещен$a в комнату имени!");
				break;

			case SCMD_UNREGISTER: pundata->duration = (times > 0) ? time(nullptr) + times * 60 * 60 : MAX_TIME;
				RegisterSystem::remove(vict);

				if (vict->in_room != kNowhere) {
					if (vict->desc && !check_dupes_host(vict->desc) && vict->in_room != r_unreg_start_room) {
						act("$n водворен$a в комнату для незарегистрированных игроков, играющих через прокси.",
							false, vict, nullptr, nullptr, kToRoom);
						RemoveCharFromRoom(vict);
						PlaceCharToRoom(vict, r_unreg_start_room);
						look_at_room(vict, r_unreg_start_room);
					}
				}
				vict->set_was_in_room(kNowhere);

				sprintf(buf, "%s unregistred by %s(%ldh).", GET_NAME(vict), GET_NAME(ch), times);
				mudlog(buf, DEF, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s", buf);
				sprintf(buf, "Unregistered (%ldh) by %s", times, GET_NAME(ch));
				AddKarma(vict, buf, reason);

				sprintf(buf, "%s%s снял$G с вас... регистрацию :).%s",
						CCIRED(vict, C_NRM), GET_NAME(ch), CCNRM(vict, C_NRM));
				sprintf(buf2, "$n лишен$a регистрации!");

				break;

		}
	}
	if (ch->in_room != kNowhere) {
		act(buf, false, vict, nullptr, ch, kToChar);
		act(buf2, false, vict, nullptr, ch, kToRoom);
	};
	return 1;
}

void is_empty_ch(ZoneRnum zone_nr, CharData *ch) {
	DescriptorData *i;
	int rnum_start, rnum_stop;
	bool found = false;

	if (room_spells::IsZoneRoomAffected(zone_nr, ESpell::kRuneLabel)) {
		SendMsgToChar("В зоне имеется рунная метка.\r\n", ch);
	}

	for (i = descriptor_list; i; i = i->next) {
		if (STATE(i) != CON_PLAYING)
			continue;
		if (i->character->in_room == kNowhere)
			continue;
		if (GetRealLevel(i->character) >= kLvlImmortal)
			continue;
		if (world[i->character->in_room]->zone_rn != zone_nr)
			continue;
		sprintf(buf2,
				"Проверка по дискрипторам: В зоне (vnum: %d клетка: %d) находится персонаж: %s.\r\n",
				zone_table[zone_nr].vnum,
				GET_ROOM_VNUM(i->character->in_room),
				GET_NAME(i->character));
		SendMsgToChar(buf2, ch);
		found = true;
	}
	if (found)
		return;
	// Поиск link-dead игроков в зонах комнаты zone_nr
	if (!GetZoneRooms(zone_nr, &rnum_start, &rnum_stop)) {
		sprintf(buf2, "Нет комнат в зоне %d.", static_cast<int>(zone_table[zone_nr].vnum));
		SendMsgToChar(buf2, ch);
		return;    // в зоне нет комнат :)
	}

	for (; rnum_start <= rnum_stop; rnum_start++) {
		// num_pc_in_room() использовать нельзя, т.к. считает вместе с иммами.
		{
			for (const auto c : world[rnum_start]->people) {
				if (!c->IsNpc() && (GetRealLevel(c) < kLvlImmortal)) {
					sprintf(buf2,
							"Проверка по списку чаров (с учетом linkdrop): в зоне vnum: %d клетка: %d находится персонаж: %s.\r\n",
							zone_table[zone_nr].vnum,
							GET_ROOM_VNUM(c->in_room),
							GET_NAME(c));
					SendMsgToChar(buf2, ch);
					found = true;
				}
			}
		}
	}

	// теперь проверю всех товарищей в void комнате STRANGE_ROOM
	for (const auto c : world[kStrangeRoom]->people) {
		int was = c->get_was_in_room();
		if (was == kNowhere
			|| GetRealLevel(c) >= kLvlImmortal
			|| world[was]->zone_rn != zone_nr) {
			continue;
		}

		sprintf(buf2,
				"В прокси руме сидит игрок %s находящийся в зоне vnum: %d клетка: %d\r\n",
				GET_NAME(c),
				zone_table[zone_nr].vnum,
				GET_ROOM_VNUM(c->in_room));
		SendMsgToChar(buf2, ch);
		found = true;
	}

	if (!found) {
		sprintf(buf2, "В зоне %d даже мышь не пробегала.\r\n", zone_table[zone_nr].vnum);
		SendMsgToChar(buf2, ch);
	}
}

void do_check_occupation(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int number;
	ZoneRnum zrn;
	one_argument(argument, buf);
	bool is_found = false;
	if (!*buf || !a_isdigit(*buf)) {
		SendMsgToChar("Usage: занятость внумзоны\r\n", ch);
		return;
	}

	if ((number = atoi(buf)) < 0) {
		SendMsgToChar("Такого внума не может быть!\r\n", ch);
		return;
	}

	// что-то по другому не нашел, как проверить существует такая зона или нет
	for (zrn = 0; zrn < static_cast<ZoneRnum>(zone_table.size()); zrn++) {
		if (zone_table[zrn].vnum == number) {
			is_empty_ch(zrn, ch);
			is_found = true;
			break;
		}
	}

	if (!is_found) {
		SendMsgToChar("Такой зоны нет.\r\n", ch);
	}
}

SetAllInspReqListType &setall_inspect_list = MUD::setall_inspect_list();

const int SETALL_FREEZE = 0;
const int SETALL_EMAIL = 1;
const int SETALL_PSWD = 2;
const int SETALL_HELL = 3;

void setall_inspect() {
	if (setall_inspect_list.size() == 0) {
		return;
	}
	auto it = setall_inspect_list.begin();
	CharData *ch = nullptr;
	DescriptorData *d_vict = nullptr;

	DescriptorData *imm_d = DescriptorByUid(player_table[it->first].unique);
	if (!imm_d
		|| (STATE(imm_d) != CON_PLAYING)
		|| !(ch = imm_d->character.get())) {
		setall_inspect_list.erase(it->first);
		return;
	}

	timeval start{}, stop{}, result{};
	int is_online;
	gettimeofday(&start, nullptr);
	Player *vict;
	for (; it->second->pos < static_cast<int>(player_table.size()); it->second->pos++) {
		vict = new Player;
		gettimeofday(&stop, nullptr);
		timediff(&result, &stop, &start);
		if (result.tv_sec > 0 || result.tv_usec >= kOptUsec) {
			delete vict;
			return;
		}
		buf1[0] = '\0';
		is_online = 0;
		d_vict = DescriptorByUid(player_table[it->second->pos].unique);
		if (d_vict)
			is_online = 1;
		if (player_table[it->second->pos].mail)
			if (strstr(player_table[it->second->pos].mail, it->second->mail)) {
				it->second->found++;
				if (it->second->type_req == SETALL_FREEZE) {
					if (is_online) {
						if (GetRealLevel(d_vict->character) >= kLvlGod) {
							sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							continue;
						}
						set_punish(imm_d->character.get(),
								   d_vict->character.get(),
								   SCMD_FREEZE,
								   it->second->reason,
								   it->second->freeze_time);
					} else {
						if (LoadPlayerCharacter(player_table[it->second->pos].name(), vict,
												ELoadCharFlags::kFindId | ELoadCharFlags::kNoCrcCheck) < 0) {
							sprintf(buf1, "Ошибка загрузки персонажа: %s.\r\n", player_table[it->second->pos].name());
							delete vict;
							it->second->out += buf1;
							continue;
						} else {
							if (GetRealLevel(vict) >= kLvlGod) {
								sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
								it->second->out += buf1;
								continue;
							}
							set_punish(imm_d->character.get(),
									   vict,
									   SCMD_FREEZE,
									   it->second->reason,
									   it->second->freeze_time);
							vict->save_char();
						}
					}
				} else if (it->second->type_req == SETALL_EMAIL) {
					if (is_online) {
						if (GetRealLevel(d_vict->character) >= kLvlGod) {
							sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							continue;
						}
						strncpy(GET_EMAIL(d_vict->character), it->second->newmail, 127);
						*(GET_EMAIL(d_vict->character) + 127) = '\0';
						sprintf(buf2,
								"Смена e-mail адреса персонажа %s с %s на %s.\r\n",
								player_table[it->second->pos].name(),
								player_table[it->second->pos].mail,
								it->second->newmail);
						AddKarma(d_vict->character.get(), buf2, GET_NAME(imm_d->character));
						it->second->out += buf2;

					} else {
						if (LoadPlayerCharacter(player_table[it->second->pos].name(), vict,
												ELoadCharFlags::kFindId | ELoadCharFlags::kNoCrcCheck) < 0) {
							sprintf(buf1, "Ошибка загрузки персонажа: %s.\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							delete vict;
							continue;
						} else {
							if (GetRealLevel(vict) >= kLvlGod) {
								it->second->out += buf1;
								continue;
							}
							strncpy(GET_EMAIL(vict), it->second->newmail, 127);
							*(GET_EMAIL(vict) + 127) = '\0';
							sprintf(buf2,
									"Смена e-mail адреса персонажа %s с %s на %s.\r\n",
									player_table[it->second->pos].name(),
									player_table[it->second->pos].mail,
									it->second->newmail);
							it->second->out += buf2;
							AddKarma(vict, buf2, GET_NAME(imm_d->character));
							vict->save_char();
						}
					}
				} else if (it->second->type_req == SETALL_PSWD) {
					if (is_online) {
						if (GetRealLevel(d_vict->character) >= kLvlGod) {
							sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							continue;
						}
						Password::set_password(d_vict->character.get(), std::string(it->second->pwd));
						sprintf(buf2, "У персонажа %s изменен пароль (setall).", player_table[it->second->pos].name());
						it->second->out += buf2;
						sprintf(buf1, "\r\n");
						it->second->out += buf1;
						AddKarma(d_vict->character.get(), buf2, GET_NAME(imm_d->character));
					} else {
						if (LoadPlayerCharacter(player_table[it->second->pos].name(), vict,
												ELoadCharFlags::kFindId | ELoadCharFlags::kNoCrcCheck) < 0) {
							sprintf(buf1, "Ошибка загрузки персонажа: %s.\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							delete vict;
							continue;
						}
						if (GetRealLevel(vict) >= kLvlGod) {
							sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							continue;
						}
						Password::set_password(vict, std::string(it->second->pwd));
						std::string str = player_table[it->second->pos].name();
						str[0] = UPPER(str[0]);
						sprintf(buf2, "У персонажа %s изменен пароль (setall).", player_table[it->second->pos].name());
						it->second->out += buf2;
						sprintf(buf1, "\r\n");
						it->second->out += buf1;
						AddKarma(vict, buf2, GET_NAME(imm_d->character));
						vict->save_char();
					}
				} else if (it->second->type_req == SETALL_HELL) {
					if (is_online) {
						if (GetRealLevel(d_vict->character) >= kLvlGod) {
							sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
							it->second->out += buf1;
							continue;
						}
						set_punish(imm_d->character.get(),
								   d_vict->character.get(),
								   SCMD_HELL,
								   it->second->reason,
								   it->second->freeze_time);
					} else {
						if (LoadPlayerCharacter(player_table[it->second->pos].name(), vict,
												ELoadCharFlags::kFindId | ELoadCharFlags::kNoCrcCheck) < 0) {
							sprintf(buf1, "Ошибка загрузки персонажа: %s.\r\n", player_table[it->second->pos].name());
							delete vict;
							it->second->out += buf1;
							continue;
						} else {
							if (GetRealLevel(vict) >= kLvlGod) {
								sprintf(buf1, "Персонаж %s бессмертный!\r\n", player_table[it->second->pos].name());
								it->second->out += buf1;
								continue;
							}
							set_punish(imm_d->character.get(),
									   vict,
									   SCMD_HELL,
									   it->second->reason,
									   it->second->freeze_time);
							vict->save_char();
						}
					}
				}
			}
		delete vict;
	}
	if (it->second->mail && it->second->pwd)
		Password::send_password(it->second->mail, it->second->pwd);
	// освобождение памяти
	if (it->second->pwd)
		free(it->second->pwd);
	if (it->second->reason)
		free(it->second->reason);
	if (it->second->newmail)
		free(it->second->newmail);
	if (it->second->mail)
		free(it->second->mail);
	gettimeofday(&stop, nullptr);
	timediff(&result, &stop, &it->second->start);
	sprintf(buf1, "Всего найдено: %d.\r\n", it->second->found);
	it->second->out += buf1;
	page_string(ch->desc, it->second->out);
	setall_inspect_list.erase(it->first);
}

void do_setall(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int type_request = 0;
	int times = 0;
	if (ch->get_pfilepos() < 0)
		return;

	auto it = setall_inspect_list.find(GET_UNIQUE(ch));
	// На всякий случай разрешаем только одну команду такого типа - либо setall, либо inspect
	if (MUD::InspectRequests().IsBusy(ch) && it != setall_inspect_list.end()) {
		SendMsgToChar(ch, "Обрабатывается другой запрос, подождите...\r\n");
		return;
	}

	argument = three_arguments(argument, buf, buf1, buf2);
	SetAllInspReqPtr req(new setall_inspect_request);
	req->newmail = nullptr;
	req->mail = nullptr;
	req->reason = nullptr;
	req->pwd = nullptr;

	if (!*buf) {
		SendMsgToChar("Usage: setall <e-mail> <email|passwd|frozen|hell> <arguments>\r\n", ch);
		return;
	}

	if (!IsValidEmail(buf)) {
		SendMsgToChar("Некорректный e-mail!\r\n", ch);
		return;
	}

	if (!isname(buf1, "frozen email passwd hell")) {
		SendMsgToChar("Данное действие совершить нельзя.\r\n", ch);
		return;
	}
	if (utils::IsAbbr(buf1, "frozen")) {
		skip_spaces(&argument);
		if (!argument || !*argument) {
			SendMsgToChar("Необходимо указать причину такой немилости.\r\n", ch);
			return;
		}
		if (*buf2) times = atol(buf2);
		type_request = SETALL_FREEZE;
		req->freeze_time = times;
		req->reason = strdup(argument);
	} else if (utils::IsAbbr(buf1, "email")) {
		if (!*buf2) {
			SendMsgToChar("Укажите новый e-mail!\r\n", ch);
			return;
		}
		if (!IsValidEmail(buf2)) {
			SendMsgToChar("Новый e-mail некорректен!\r\n", ch);
			return;
		}
		req->newmail = strdup(buf2);
		type_request = SETALL_EMAIL;
	} else if (utils::IsAbbr(buf1, "passwd")) {
		if (!*buf2) {
			SendMsgToChar("Укажите новый пароль!\r\n", ch);
			return;
		}
		req->pwd = strdup(buf2);
		type_request = SETALL_PSWD;
	} else if (utils::IsAbbr(buf1, "hell")) {
		skip_spaces(&argument);
		if (!argument || !*argument) {
			SendMsgToChar("Необходимо указать причину такой немилости.\r\n", ch);
			return;
		}
		if (*buf2) times = atol(buf2);
		type_request = SETALL_HELL;
		req->freeze_time = times;
		req->reason = strdup(argument);
	} else {
		SendMsgToChar("Какой-то баг. Вы эту надпись видеть не должны.\r\n", ch);
		return;
	}

	req->type_req = type_request;
	req->mail = str_dup(buf);
	req->pos = 0;
	req->found = 0;
	req->out = "";
	setall_inspect_list[ch->get_pfilepos()] = req;
}

void do_echo(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	if (ch->IsFlagged(EPlrFlag::kDumbed)) {
		SendMsgToChar("Вы не в состоянии что-либо продемонстрировать окружающим.\r\n", ch);
		return;
	}

	skip_spaces(&argument);

	if (!*argument) {
		SendMsgToChar("И что вы хотите выразить столь красочно?\r\n", ch);
	} else {
		if (subcmd == SCMD_EMOTE) {
			if (ch->IsNpc() && AFF_FLAGGED(ch, EAffect::kCharmed)) {
				if (ch->get_master()->IsFlagged(EPlrFlag::kDumbed)) {
					SendMsgToChar("Ваши последователи так же немы, как и вы!\r\n", ch->get_master());
					return;
				}
			}
			sprintf(buf, "&K$n %s.&n", argument);
		} else {
			strcpy(buf, argument);
		}

		for (const auto to : world[ch->in_room]->people) {
			if (to == ch
				|| ignores(to, ch, EIgnore::kEmote)) {
				continue;
			}

			act(buf, false, ch, nullptr, to, kToVict | kToNotDeaf);
			act(deaf_social, false, ch, nullptr, to, kToVict | kToDeaf);
		}

		if (ch->IsFlagged(EPrf::kNoRepeat)) {
			SendMsgToChar(OK, ch);
		} else {
			act(buf, false, ch, nullptr, nullptr, kToChar);
		}
	}
}

#define SHOW_GLORY    0
#define ADD_GLORY    1
#define SUB_GLORY    2
#define SUB_STATS    3
#define SUB_TRANS    4
#define SUB_HIDE    5

void do_glory(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	// Команда простановки славы (оффлайн/онлайн)
	// Без параметров выводит славу у игрока
	// + cлава прибавляет славу
	// - cлава убавляет славу
	char num[kMaxInputLength];
	char arg1[kMaxInputLength];
	int mode = 0;
	char *reason;

	if (!*argument) {
		SendMsgToChar("Формат команды : \r\n"
					 "   glory <имя> +|-<кол-во славы> причина\r\n"
					 "   glory <имя> remove <кол-во статов> причина (снимание уже вложенной чаром славы)\r\n"
					 "   glory <имя> transfer <имя принимаюего славу> причина (переливание славы на другого чара)\r\n"
					 "   glory <имя> hide on|off причина (показывать или нет чара в топе славы)\r\n", ch);
		return;
	}
	reason = two_arguments(argument, arg, num);
	skip_spaces(&reason);

	if (!*num)
		mode = SHOW_GLORY;
	else if (*num == '+')
		mode = ADD_GLORY;
	else if (*num == '-')
		mode = SUB_GLORY;
	else if (utils::IsAbbr(num, "remove")) {
		// тут у нас в num получается remove, в arg1 кол-во и в reason причина
		reason = one_argument(reason, arg1);
		skip_spaces(&reason);
		mode = SUB_STATS;
	} else if (utils::IsAbbr(num, "transfer")) {
		// а тут в num transfer, в arg1 имя принимающего славу и в reason причина
		reason = one_argument(reason, arg1);
		skip_spaces(&reason);
		mode = SUB_TRANS;
	} else if (utils::IsAbbr(num, "hide")) {
		// а тут в num hide, в arg1 on|off и в reason причина
		reason = any_one_arg(reason, arg1);
		skip_spaces(&reason);
		mode = SUB_HIDE;
	}

	// точки убираем, чтобы карма всегда писалась
	skip_dots(&reason);

	if (mode != SHOW_GLORY) {
		if ((reason == nullptr) || (*reason == 0)) {
			SendMsgToChar("Укажите причину изменения славы?\r\n", ch);
			return;
		}
	}

	CharData *vict = get_player_vis(ch, arg, EFind::kCharInWorld);
	Player t_vict; // TODO: надо выносить во вторую функцию, чтобы зря не создавать
	if (!vict) {
		if (LoadPlayerCharacter(arg, &t_vict, ELoadCharFlags::kFindId) < 0) {
			SendMsgToChar("Такого персонажа не существует.\r\n", ch);
			return;
		}
		vict = &t_vict;
	}

	switch (mode) {
		case ADD_GLORY: {
			int amount = atoi((num + 1));
			Glory::add_glory(GET_UNIQUE(vict), amount);
			SendMsgToChar(ch, "%s добавлено %d у.е. славы (Всего: %d у.е.).\r\n",
						  GET_PAD(vict, 2), amount, Glory::get_glory(GET_UNIQUE(vict)));
			imm_log("(GC) %s sets +%d glory to %s.", GET_NAME(ch), amount, GET_NAME(vict));
			// запись в карму
			sprintf(buf, "Change glory +%d by %s", amount, GET_NAME(ch));
			AddKarma(vict, buf, reason);
			GloryMisc::add_log(mode, amount, std::string(buf), std::string(reason), vict);
			break;
		}
		case SUB_GLORY: {
			int amount = Glory::remove_glory(GET_UNIQUE(vict), atoi((num + 1)));
			if (amount <= 0) {
				SendMsgToChar(ch, "У %s нет свободной славы.", GET_PAD(vict, 1));
				break;
			}
			SendMsgToChar(ch, "У %s вычтено %d у.е. славы (Всего: %d у.е.).\r\n",
						  GET_PAD(vict, 1), amount, Glory::get_glory(GET_UNIQUE(vict)));
			imm_log("(GC) %s sets -%d glory to %s.", GET_NAME(ch), amount, GET_NAME(vict));
			// запись в карму
			sprintf(buf, "Change glory -%d by %s", amount, GET_NAME(ch));
			AddKarma(vict, buf, reason);
			GloryMisc::add_log(mode, amount, std::string(buf), std::string(reason), vict);
			break;
		}
		case SUB_STATS: {
			if (Glory::remove_stats(vict, ch, atoi(arg1))) {
				sprintf(buf, "Remove stats %s by %s", arg1, GET_NAME(ch));
				AddKarma(vict, buf, reason);
				GloryMisc::add_log(mode, 0, std::string(buf), std::string(reason), vict);
			}
			break;
		}
		case SUB_TRANS: {
			Glory::transfer_stats(vict, ch, arg1, reason);
			break;
		}
		case SUB_HIDE: {
			Glory::hide_char(vict, ch, arg1);
			sprintf(buf, "Hide %s by %s", arg1, GET_NAME(ch));
			AddKarma(vict, buf, reason);
			GloryMisc::add_log(mode, 0, std::string(buf), std::string(reason), vict);
			break;
		}
		default: Glory::show_glory(vict, ch);
	}

	vict->save_char();
}

void do_send(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *vict;

	half_chop(argument, arg, buf);

	if (!*arg) {
		SendMsgToChar("Послать что и кому (не путать с куда и кого :)\r\n", ch);
		return;
	}
	if (!(vict = get_player_vis(ch, arg, EFind::kCharInWorld))) {
		SendMsgToChar(NOPERSON, ch);
		return;
	}
	SendMsgToChar(buf, vict);
	SendMsgToChar("\r\n", vict);
	if (ch->IsFlagged(EPrf::kNoRepeat))
		SendMsgToChar("Послано.\r\n", ch);
	else {
		snprintf(buf2, kMaxStringLength, "Вы послали '%s' %s.\r\n", buf, GET_PAD(vict, 2));
		SendMsgToChar(buf2, ch);
	}
}

// Take a string, and return a rnum. Used for goto, at, etc.  -je 4/6/93
RoomRnum find_target_room(CharData *ch, char *rawroomstr, int trig) {
	RoomVnum tmp;
	RoomRnum location;
	CharData *target_mob;
	ObjData *target_obj;
	char roomstr[kMaxInputLength];

	one_argument(rawroomstr, roomstr);

	if (!*roomstr) {
		SendMsgToChar("Укажите номер или название комнаты.\r\n", ch);
		return (kNowhere);
	}
	if (a_isdigit(*roomstr) && !strchr(roomstr, '.')) {
		tmp = atoi(roomstr);
		if ((location = GetRoomRnum(tmp)) == kNowhere) {
			SendMsgToChar("Нет комнаты с таким номером.\r\n", ch);
			return (kNowhere);
		}
	} else if ((target_mob = get_char_vis(ch, roomstr, EFind::kCharInWorld)) != nullptr) {
		location = target_mob->in_room;
	} else if ((target_obj = get_obj_vis(ch, roomstr)) != nullptr) {
		if (target_obj->get_in_room() != kNowhere) {
			location = target_obj->get_in_room();
		} else {
			SendMsgToChar("Этот объект вам недоступен.\r\n", ch);
			return (kNowhere);
		}
	} else {
		SendMsgToChar("В округе нет похожего предмета или создания.\r\n", ch);
		return (kNowhere);
	}

	// a location has been found -- if you're < GRGOD, check restrictions.
	if (!IS_GRGOD(ch) && !ch->IsFlagged(EPrf::kCoderinfo)) {
		if (ROOM_FLAGGED(location, ERoomFlag::kGodsRoom) && GetRealLevel(ch) < kLvlGreatGod) {
			SendMsgToChar("Вы не столь божественны, чтобы получить доступ в эту комнату!\r\n", ch);
			return (kNowhere);
		}
		if (ROOM_FLAGGED(location, ERoomFlag::kNoTeleportIn) && trig != 1) {
			SendMsgToChar("В комнату не телепортировать!\r\n", ch);
			return (kNowhere);
		}
		if (!Clan::MayEnter(ch, location, kHousePortal)) {
			SendMsgToChar("Частная собственность - посторонним в ней делать нечего!\r\n", ch);
			return (kNowhere);
		}
	}
	return (location);
}

void do_at(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	char command[kMaxInputLength];
	RoomRnum location, original_loc;

	half_chop(argument, buf, command);
	if (!*buf) {
		SendMsgToChar("Необходимо указать номер или название комнаты.\r\n", ch);
		return;
	}

	if (!*command) {
		SendMsgToChar("Что вы собираетесь там делать?\r\n", ch);
		return;
	}

	if ((location = find_target_room(ch, buf, 0)) == kNowhere)
		return;

	// a location has been found.
	original_loc = ch->in_room;
	RemoveCharFromRoom(ch);
	PlaceCharToRoom(ch, location);
	command_interpreter(ch, command);

	// check if the char is still there
	if (ch->in_room == location) {
		RemoveCharFromRoom(ch);
		PlaceCharToRoom(ch, original_loc);
	}
	ch->dismount();
}

void do_unfreeze(CharData *ch, char * /*argument*/, int/* cmd*/, int/* subcmd*/) {
	/*Формат файл unfreeze.lst
	Первая строка email
	Вторая строка причина по которой разфриз
	Все остальные строки полные имена чаров*/
	//char email[50], reason[50];
	Player t_vict;
	CharData *vict;
	char *reason_c; // для функции set_punish, она не умеет принимать тип string :(
	std::string email;
	std::string reason;
	std::string name_buffer;
	ifstream unfreeze_list;
	unfreeze_list.open("../lib/misc/unfreeze.lst", fstream::in);
	if (!unfreeze_list) {
		SendMsgToChar("Файл unfreeze.lst отсутствует!\r\n", ch);
		return;
	}
	unfreeze_list >> email;
	unfreeze_list >> reason;
	sprintf(buf, "Начинаем масс.разфриз\r\nEmail:%s\r\nПричина:%s\r\n", email.c_str(), reason.c_str());
	SendMsgToChar(buf, ch);
	reason_c = new char[reason.length() + 1];
	strcpy(reason_c, reason.c_str());

	while (!unfreeze_list.eof()) {
		unfreeze_list >> name_buffer;
		if (LoadPlayerCharacter(name_buffer.c_str(), &t_vict, ELoadCharFlags::kFindId) < 0) {
			sprintf(buf, "Чара с именем %s не существует !\r\n", name_buffer.c_str());
			SendMsgToChar(buf, ch);
			continue;
		}
		vict = &t_vict;
		if (GET_EMAIL(vict) != email) {
			sprintf(buf, "У чара %s другой емайл.\r\n", name_buffer.c_str());
			SendMsgToChar(buf, ch);
			continue;
		}
		set_punish(ch, vict, SCMD_FREEZE, reason_c, 0);
		vict->save_char();
		sprintf(buf, "Чар %s разморожен.\r\n", name_buffer.c_str());
		SendMsgToChar(buf, ch);
	}

	delete[] reason_c;
	unfreeze_list.close();

}

void do_goto(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	RoomRnum location;

	if ((location = find_target_room(ch, argument, 0)) == kNowhere)
		return;

	if (POOFOUT(ch))
		sprintf(buf, "$n %s", POOFOUT(ch));
	else
		strcpy(buf, "$n растворил$u в клубах дыма.");

	act(buf, true, ch, nullptr, nullptr, kToRoom);
	RemoveCharFromRoom(ch);
	PlaceCharToRoom(ch, location);
	ch->dismount();

	if (POOFIN(ch))
		sprintf(buf, "$n %s", POOFIN(ch));
	else
		strcpy(buf, "$n возник$q посреди комнаты.");
	act(buf, true, ch, nullptr, nullptr, kToRoom);
	look_at_room(ch, 0);
}

void do_teleport(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *victim;
	RoomRnum target;

	two_arguments(argument, buf, buf2);

	if (!*buf)
		SendMsgToChar("Кого вы хотите переместить?\r\n", ch);
	else if (!(victim = get_char_vis(ch, buf, EFind::kCharInWorld)))
		SendMsgToChar(NOPERSON, ch);
	else if (victim == ch)
		SendMsgToChar("Используйте 'прыжок' для собственного перемещения.\r\n", ch);
	else if (GetRealLevel(victim) >= GetRealLevel(ch) && !ch->IsFlagged(EPrf::kCoderinfo))
		SendMsgToChar("Попробуйте придумать что-то другое.\r\n", ch);
	else if (!*buf2)
		act("Куда вы хотите $S переместить?", false, ch, nullptr, victim, kToChar);
	else if ((target = find_target_room(ch, buf2, 0)) != kNowhere) {
		SendMsgToChar(OK, ch);
		act("$n растворил$u в клубах дыма.", false, victim, nullptr, nullptr, kToRoom);
		RemoveCharFromRoom(victim);
		PlaceCharToRoom(victim, target);
		victim->dismount();
		act("$n появил$u, окутанн$w розовым туманом.",
			false, victim, nullptr, nullptr, kToRoom);
		act("$n переместил$g вас!", false, ch, nullptr, (char *) victim, kToVict);
		look_at_room(victim, 0);
	}
}

void do_shutdown(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	commands::Shutdown command(ch, argument, shutdown_parameters);
	if (command.parse_arguments()) {
		command.execute();
	}
}

void stop_snooping(CharData *ch) {
	if (!ch->desc->snooping)
		SendMsgToChar("Вы не подслушиваете.\r\n", ch);
	else {
		SendMsgToChar("Вы прекратили подслушивать.\r\n", ch);
		ch->desc->snooping->snoop_by = nullptr;
		ch->desc->snooping = nullptr;
	}
}

void do_snoop(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *victim, *tch;

	if (!ch->desc)
		return;

	argument = one_argument(argument, arg);

	if (!*arg)
		stop_snooping(ch);
	else if (!(victim = get_player_vis(ch, arg, EFind::kCharInWorld)))
		SendMsgToChar("Нет такого создания в игре.\r\n", ch);
	else if (!victim->desc)
		act("Вы не можете $S подслушать - он$G потерял$G связь..\r\n",
			false, ch, nullptr, victim, kToChar);
	else if (victim == ch)
		stop_snooping(ch);
	else if (victim->desc->snooping == ch->desc)
		SendMsgToChar("Вы уже подслушиваете.\r\n", ch);
	else if (victim->desc->snoop_by && victim->desc->snoop_by != ch->desc)
		SendMsgToChar("Дык его уже кто-то из богов подслушивает.\r\n", ch);
	else {
		if (victim->desc->original)
			tch = victim->desc->original.get();
		else
			tch = victim;

		const int god_level = ch->IsFlagged(EPrf::kCoderinfo) ? kLvlImplementator : GetRealLevel(ch);
		const int victim_level = tch->IsFlagged(EPrf::kCoderinfo) ? kLvlImplementator : GetRealLevel(tch);

		if (victim_level >= god_level) {
			SendMsgToChar("Вы не можете.\r\n", ch);
			return;
		}
		SendMsgToChar(OK, ch);

		ch->desc->snoop_with_map = false;
		if (god_level >= kLvlImplementator && argument && *argument) {
			skip_spaces(&argument);
			if (isname(argument, "map") || isname(argument, "карта")) {
				ch->desc->snoop_with_map = true;
			}
		}

		if (ch->desc->snooping)
			ch->desc->snooping->snoop_by = nullptr;

		ch->desc->snooping = victim->desc;
		victim->desc->snoop_by = ch->desc;
	}
}

void do_switch(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	one_argument(argument, arg);

	if (ch->desc->original) {
		SendMsgToChar("Вы уже в чьем-то теле.\r\n", ch);
	} else if (!*arg) {
		SendMsgToChar("Стать кем?\r\n", ch);
	} else {
		const auto visible_character = get_char_vis(ch, arg, EFind::kCharInWorld);
		if (!visible_character) {
			SendMsgToChar("Нет такого создания.\r\n", ch);
		} else if (ch == visible_character) {
			SendMsgToChar("Вы и так им являетесь.\r\n", ch);
		} else if (visible_character->desc) {
			SendMsgToChar("Это тело уже под контролем.\r\n", ch);
		} else if (!IS_IMPL(ch)
			&& !visible_character->IsNpc()) {
			SendMsgToChar("Вы не столь могущественны, чтобы контроолировать тело игрока.\r\n", ch);
		} else if (GetRealLevel(ch) < kLvlGreatGod
			&& ROOM_FLAGGED(visible_character->in_room, ERoomFlag::kGodsRoom)) {
			SendMsgToChar("Вы не можете находиться в той комнате.\r\n", ch);
		} else if (!IS_GRGOD(ch)
			&& !Clan::MayEnter(ch, visible_character->in_room, kHousePortal)) {
			SendMsgToChar("Вы не сможете проникнуть на частную территорию.\r\n", ch);
		} else {
			const auto victim = character_list.get_character_by_address(visible_character);
			const auto me = character_list.get_character_by_address(ch);
			if (!victim || !me) {
				SendMsgToChar("Something went wrong. Report this bug to developers\r\n", ch);
				return;
			}

			SendMsgToChar(OK, ch);

			ch->desc->character = victim;
			ch->desc->original = me;

			victim->desc = ch->desc;
			ch->desc = nullptr;
		}
	}
}

void do_return(CharData *ch, char *argument, int cmd, int subcmd) {
	if (ch->desc && ch->desc->original) {
		SendMsgToChar("Вы вернулись в свое тело.\r\n", ch);

		/*
		 * If someone switched into your original body, disconnect them.
		 *   - JE 2/22/95
		 *
		 * Zmey: here we put someone switched in our body to disconnect state
		 * but we must also NULL his pointer to our character, otherwise
		 * close_socket() will damage our character's pointer to our descriptor
		 * (which is assigned below in this function). 12/17/99
		 */
		if (ch->desc->original->desc) {
			ch->desc->original->desc->character = nullptr;
			STATE(ch->desc->original->desc) = CON_DISCONNECT;
		}
		ch->desc->character = ch->desc->original;
		ch->desc->original = nullptr;

		ch->desc->character->desc = ch->desc;
		ch->desc = nullptr;
	} else {
		do_recall(ch, argument, cmd, subcmd);
	}
}

void do_load(CharData *ch, char *argument, int cmd, int/* subcmd*/) {
	CharData *mob;
	MobVnum number;
	MobRnum r_num;
	char *iname;

	iname = two_arguments(argument, buf, buf2);

	if (!(privilege::HasPrivilege(ch, std::string(cmd_info[cmd].command), 0, 0, false)) && (GET_OLC_ZONE(ch) <= 0)) {
		SendMsgToChar("Чаво?\r\n", ch);
		return;
	}
	int first = atoi(buf2) / 100;

	if (!IS_IMMORTAL(ch) && GET_OLC_ZONE(ch) != first) {
		SendMsgToChar("Доступ к данной зоне запрещен!\r\n", ch);
		return;
	}
	if (!*buf || !*buf2 || !a_isdigit(*buf2)) {
		SendMsgToChar("Usage: load { obj | mob } <number>\r\n"
					  "       load ing { <сила> | <VNUM> } <имя>\r\n", ch);
		return;
	}
	if ((number = atoi(buf2)) < 0) {
		SendMsgToChar("Отрицательный моб опасен для вашего здоровья!\r\n", ch);
		return;
	}
	if (utils::IsAbbr(buf, "mob")) {
		if ((r_num = GetMobRnum(number)) < 0) {
			SendMsgToChar("Нет такого моба в этом МУДе.\r\n", ch);
			return;
		}
		if ((zone_table[get_zone_rnum_by_mob_vnum(number)].locked) && (GetRealLevel(ch) != kLvlImplementator)) {
			SendMsgToChar("Зона защищена от записи. С вопросами к старшим богам.\r\n", ch);
			return;
		}
		mob = ReadMobile(r_num, kReal);
		PlaceCharToRoom(mob, ch->in_room);
		act("$n порыл$u в МУДе.", true, ch, nullptr, nullptr, kToRoom);
		act("$n создал$g $N3!", false, ch, nullptr, mob, kToRoom);
		act("Вы создали $N3.", false, ch, nullptr, mob, kToChar);
		load_mtrigger(mob);
		olc_log("%s load mob %s #%d", GET_NAME(ch), GET_NAME(mob), number);
	} else if (utils::IsAbbr(buf, "obj")) {
		if ((r_num = GetObjRnum(number)) < 0) {
			SendMsgToChar("Господи, да изучи ты номера объектов.\r\n", ch);
			return;
		}
		if ((zone_table[get_zone_rnum_by_obj_vnum(number)].locked) && (GetRealLevel(ch) != kLvlImplementator)) {
			SendMsgToChar("Зона защищена от записи. С вопросами к старшим богам.\r\n", ch);
			return;
		}
		const auto obj = world_objects.create_from_prototype_by_rnum(r_num);
		obj->set_crafter_uid(GET_UNIQUE(ch));
		obj->set_vnum_zone_from(GetZoneVnumByCharPlace(ch));

		if (number == GlobalDrop::MAGIC1_ENCHANT_VNUM
			|| number == GlobalDrop::MAGIC2_ENCHANT_VNUM
			|| number == GlobalDrop::MAGIC3_ENCHANT_VNUM) {
			generate_magic_enchant(obj.get());
		}

		if (load_into_inventory) {
			PlaceObjToInventory(obj.get(), ch);
		} else {
			PlaceObjToRoom(obj.get(), ch->in_room);
		}

		act("$n покопал$u в МУДе.", true, ch, nullptr, nullptr, kToRoom);
		act("$n создал$g $o3!", false, ch, obj.get(), nullptr, kToRoom);
		act("Вы создали $o3.", false, ch, obj.get(), nullptr, kToChar);
		load_otrigger(obj.get());
		CheckObjDecay(obj.get());
		olc_log("%s load obj %s #%d", GET_NAME(ch), obj->get_short_description().c_str(), number);
	} else if (utils::IsAbbr(buf, "ing")) {
		int power, i;
		power = atoi(buf2);
		skip_spaces(&iname);
		i = im_get_type_by_name(iname, 0);
		if (i < 0) {
			SendMsgToChar("Неверное имя типа\r\n", ch);
			return;
		}
		const auto obj = load_ingredient(i, power, power);
		if (!obj) {
			SendMsgToChar("Ошибка загрузки ингредиента\r\n", ch);
			return;
		}
		PlaceObjToInventory(obj, ch);
		act("$n покопал$u в МУДе.", true, ch, nullptr, nullptr, kToRoom);
		act("$n создал$g $o3!", false, ch, obj, nullptr, kToRoom);
		act("Вы создали $o3.", false, ch, obj, nullptr, kToChar);
		sprintf(buf, "%s load ing %d %s", GET_NAME(ch), power, iname);
		mudlog(buf, NRM, kLvlBuilder, IMLOG, true);
		load_otrigger(obj);
		CheckObjDecay(obj);
		olc_log("%s load ing %s #%d", GET_NAME(ch), obj->get_short_description().c_str(), power);
	} else {
		SendMsgToChar("Нет уж. Ты создай че-нить нормальное.\r\n", ch);
	}
}

// отправка сообщения вообще всем
void send_to_all(char *buffer) {
	DescriptorData *pt;
	for (pt = descriptor_list; pt; pt = pt->next) {
		if (STATE(pt) == CON_PLAYING && pt->character) {
			SendMsgToChar(buffer, pt->character.get());
		}
	}
}

void do_vstat(CharData *ch, char *argument, int cmd, int/* subcmd*/) {
	CharData *mob;
	MobVnum number;    // or ObjVnum ...
	MobRnum r_num;        // or ObjRnum ...

	two_arguments(argument, buf, buf2);
	int first = atoi(buf2) / 100;

	if (!(privilege::HasPrivilege(ch, std::string(cmd_info[cmd].command), 0, 0, false)) && (GET_OLC_ZONE(ch) <= 0)) {
		SendMsgToChar("Чаво?\r\n", ch);
		return;
	}

	if (!*buf || !*buf2 || !a_isdigit(*buf2)) {
		SendMsgToChar("Usage: vstat { obj | mob } <number>\r\n", ch);
		return;
	}


	if (!IS_IMMORTAL(ch) && GET_OLC_ZONE(ch) != first) {
		SendMsgToChar("Доступ к данной зоне запрещен!\r\n", ch);
		return;
	}

	if ((number = atoi(buf2)) < 0) {
		SendMsgToChar("Отрицательный номер? Оригинально!\r\n", ch);
		return;
	}
	if (utils::IsAbbr(buf, "mob")) {
		if ((r_num = GetMobRnum(number)) < 0) {
			SendMsgToChar("Обратитесь в Арктику - там ОН живет.\r\n", ch);
			return;
		}
		mob = ReadMobile(r_num, kReal);
		PlaceCharToRoom(mob, 1);
		do_stat_character(ch, mob, 1);
		ExtractCharFromWorld(mob, false);
	} else if (utils::IsAbbr(buf, "obj")) {
		if ((r_num = GetObjRnum(number)) < 0) {
			SendMsgToChar("Этот предмет явно перенесли в РМУД.\r\n", ch);
			return;
		}

		const auto obj = world_objects.create_from_prototype_by_rnum(r_num);
		do_stat_object(ch, obj.get(), 1);
		ExtractObjFromWorld(obj.get());
	} else
		SendMsgToChar("Тут должно быть что-то типа 'obj' или 'mob'.\r\n", ch);
}

// clean a room of all mobiles and objects
void do_purge(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *vict;
	ObjData *obj, *next_o;

	one_argument(argument, buf);

	if (*buf) {        // argument supplied. destroy single object or char
		if ((vict = get_char_vis(ch, buf, EFind::kCharInRoom)) != nullptr) {
			if (!vict->IsNpc() && GetRealLevel(ch) <= GetRealLevel(vict) && !ch->IsFlagged(EPrf::kCoderinfo)) {
				SendMsgToChar("Да я вас за это...\r\n", ch);
				return;
			}
			act("$n обратил$g в прах $N3.", false, ch, nullptr, vict, kToNotVict);
			if (!vict->IsNpc()) {
				sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
				mudlog(buf, CMP, std::max(kLvlImmortal, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("%s has purged %s.", GET_NAME(ch), GET_NAME(vict));
				if (vict->desc) {
					STATE(vict->desc) = CON_CLOSE;
					vict->desc->character = nullptr;
					vict->desc = nullptr;
				}
			}
			// TODO: честно говоря дублирование куска из экстракта не ясно
			// смену лидера пока сюду не сую, над вникнуть будет...
			if (vict->followers
				|| vict->has_master()) {
				die_follower(vict);
			}

			if (!vict->purged()) {
				ExtractCharFromWorld(vict, false);
			}
		} else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room]->contents)) != nullptr) {
			act("$n просто разметал$g $o3 на молекулы.", false, ch, obj, nullptr, kToRoom);
			ExtractObjFromWorld(obj);
		} else {
			SendMsgToChar("Ничего похожего с таким именем нет.\r\n", ch);
			return;
		}
		SendMsgToChar(OK, ch);
	} else        // no argument. clean out the room
	{
		act("$n произнес$q СЛОВО... вас окружило пламя!", false, ch, nullptr, nullptr, kToRoom);
		SendMsgToRoom("Мир стал немного чище.\r\n", ch->in_room, false);
		for (obj = world[ch->in_room]->contents; obj; obj = next_o) { //сначала шмотки, иначе потетеряешь весь стаф с случайных чармисов
			next_o = obj->get_next_content();
			ExtractObjFromWorld(obj);
		}
		const auto people_copy = world[ch->in_room]->people;
		for (const auto vict : people_copy) {
			if (vict->IsNpc()) {
				if (vict->followers
					|| vict->has_master()) {
					die_follower(vict);
				}
				if (!vict->purged()) {
					ExtractCharFromWorld(vict, false);
				}
			}
		}
	}
}

const char *logtypes[] =
	{
		"нет", "начальный", "краткий", "нормальный", "полный", "\n"
	};

// subcmd - канал
void do_syslog(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	int tp;

	if (subcmd < 0 || subcmd > LAST_LOG) {
		return;
	}

	tp = GET_LOGS(ch)[subcmd];
	if (tp > 4)
		tp = 4;
	if (tp < 0)
		tp = 0;

	one_argument(argument, arg);

	if (*arg) {
		if (GetRealLevel(ch) == kLvlImmortal)
			logtypes[2] = "\n";
		else
			logtypes[2] = "краткий";
		if (GetRealLevel(ch) == kLvlGod)
			logtypes[4] = "\n";
		else
			logtypes[4] = "полный";
		if ((tp = search_block(arg, logtypes, false)) == -1) {
			if (GetRealLevel(ch) == kLvlImmortal)
				SendMsgToChar("Формат: syslog { нет | начальный }\r\n", ch);
			else if (GetRealLevel(ch) == kLvlGod)
				SendMsgToChar("Формат: syslog { нет | начальный | краткий | нормальный }\r\n", ch);
			else
				SendMsgToChar
					("Формат: syslog { нет | начальный | краткий | нормальный | полный }\r\n", ch);
			return;
		}
		GET_LOGS(ch)[subcmd] = tp;
	}
	sprintf(buf,
			"Тип вашего лога (%s) сейчас %s.\r\n",
			runtime_config.logs(static_cast<EOutputStream>(subcmd)).title().c_str(),
			logtypes[tp]);
	SendMsgToChar(buf, ch);
}

void do_advance(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	CharData *victim;
	char *name = arg, *level = buf2;
	int newlevel, oldlevel;

	two_arguments(argument, name, level);

	if (*name) {
		if (!(victim = get_player_vis(ch, name, EFind::kCharInWorld))) {
			SendMsgToChar("Не найду такого игрока.\r\n", ch);
			return;
		}
	} else {
		SendMsgToChar("Повысить кого?\r\n", ch);
		return;
	}

	if (GetRealLevel(ch) <= GetRealLevel(victim) && !ch->IsFlagged(EPrf::kCoderinfo)) {
		SendMsgToChar("Нелогично.\r\n", ch);
		return;
	}
	if (!*level || (newlevel = atoi(level)) <= 0) {
		SendMsgToChar("Это не похоже на уровень.\r\n", ch);
		return;
	}
	if (newlevel > kLvlImplementator) {
		sprintf(buf, "%d - максимальный возможный уровень.\r\n", kLvlImplementator);
		SendMsgToChar(buf, ch);
		return;
	}
	if (newlevel > GetRealLevel(ch) && !ch->IsFlagged(EPrf::kCoderinfo)) {
		SendMsgToChar("Вы не можете установить уровень выше собственного.\r\n", ch);
		return;
	}
	if (newlevel == GetRealLevel(victim)) {
		act("$E и так этого уровня.", false, ch, nullptr, victim, kToChar);
		return;
	}
	oldlevel = GetRealLevel(victim);
	if (newlevel < oldlevel) {
		SendMsgToChar("Вас окутало облако тьмы.\r\n" "Вы почувствовали себя лишенным чего-то.\r\n", victim);
	} else {
		act("$n сделал$g несколько странных пасов.\r\n"
			"Вам показалось, будто неземное тепло разлилось по каждой клеточке\r\n"
			"Вашего тела, наполняя его доселе невиданными вами ощущениями.\r\n",
			false, ch, nullptr, victim, kToVict);
	}

	SendMsgToChar(OK, ch);
	if (newlevel < oldlevel) {
		log("(GC) %s demoted %s from level %d to %d.", GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
		imm_log("%s demoted %s from level %d to %d.", GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
	} else {
		log("(GC) %s has advanced %s to level %d (from %d)",
			GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
		imm_log("%s has advanced %s to level %d (from %d)", GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
	}

	gain_exp_regardless(victim, GetExpUntilNextLvl(victim, newlevel)
		- GET_EXP(victim));
	victim->save_char();
}

void do_restore(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	CharData *vict;

	one_argument(argument, buf);
	if (!*buf)
		SendMsgToChar("Кого вы хотите восстановить?\r\n", ch);
	else if (!(vict = get_char_vis(ch, buf, EFind::kCharInWorld)))
		SendMsgToChar(NOPERSON, ch);
	else {
		// имм с привилегией arena может ресторить только чаров, находящихся с ним на этой же арене
		// плюс исключается ситуация, когда они в одной зоне, но чар не в клетке арены
		if (privilege::CheckFlag(ch, privilege::kArenaMaster)) {
			if (!ROOM_FLAGGED(vict->in_room, ERoomFlag::kArena) || world[ch->in_room]->zone_rn != world[vict->in_room]->zone_rn) {
				SendMsgToChar("Не положено...\r\n", ch);
				return;
			}
		}

		GET_HIT(vict) = GET_REAL_MAX_HIT(vict);
		GET_MOVE(vict) = GET_REAL_MAX_MOVE(vict);
		if (IS_MANA_CASTER(vict)) {
			vict->mem_queue.stored = GET_MAX_MANA(vict);
		} else {
			vict->mem_queue.stored = vict->mem_queue.total;
		}
		if (vict->GetSkill(ESkill::kWarcry) > 0) {
			struct TimedSkill wctimed;
			wctimed.skill = ESkill::kWarcry;
			wctimed.time = 0;
			ImposeTimedSkill(vict, &wctimed);
		}
		if (IS_GRGOD(ch) && IS_IMMORTAL(vict)) {
			vict->set_str(25);
			vict->set_int(25);
			vict->set_wis(25);
			vict->set_dex(25);
			vict->set_con(25);
			vict->set_cha(25);
		}
		update_pos(vict);
		RemoveAffectFromChar(vict, ESpell::kDrunked);
		GET_DRUNK_STATE(vict) = GET_COND(vict, DRUNK) = 0;
		RemoveAffectFromChar(vict, ESpell::kAbstinent);

		//сброс таймеров скиллов и фитов
		while (vict->timed)
			ExpireTimedSkill(vict, vict->timed);
		while (vict->timed_feat)
			ExpireTimedFeat(vict, vict->timed_feat);

		if (subcmd == SCMD_RESTORE_GOD) {
			SendMsgToChar(OK, ch);
			act("Вы были полностью восстановлены $N4!",
				false, vict, nullptr, ch, kToChar);
		}
		affect_total(vict);
	}
}

void perform_immort_vis(CharData *ch) {
	if (GET_INVIS_LEV(ch) == 0 &&
		!AFF_FLAGGED(ch, EAffect::kHide) && !AFF_FLAGGED(ch, EAffect::kInvisible)
		&& !AFF_FLAGGED(ch, EAffect::kDisguise)) {
		SendMsgToChar("Ну вот вас и заметили. Стало ли вам легче от этого?\r\n", ch);
		return;
	}

	SET_INVIS_LEV(ch, 0);
	appear(ch);
	SendMsgToChar("Вы теперь полностью видны.\r\n", ch);
}

void perform_immort_invis(CharData *ch, int level) {
	if (ch->IsNpc()) {
		return;
	}

	for (const auto tch : world[ch->in_room]->people) {
		if (tch == ch) {
			continue;
		}

		if (GetRealLevel(tch) >= GET_INVIS_LEV(ch) && GetRealLevel(tch) < level) {
			act("Вы вздрогнули, когда $n растворил$u на ваших глазах.",
				false, ch, nullptr, tch, kToVict);
		}

		if (GetRealLevel(tch) < GET_INVIS_LEV(ch) && GetRealLevel(tch) >= level) {
			act("$n медленно появил$u из пустоты.",
				false, ch, nullptr, tch, kToVict);
		}
	}

	SET_INVIS_LEV(ch, level);
	sprintf(buf, "Ваш уровень невидимости - %d.\r\n", level);
	SendMsgToChar(buf, ch);
}

void do_invis(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int level;

	if (ch->IsNpc()) {
		SendMsgToChar("Вы не можете сделать этого.\r\n", ch);
		return;
	}

	one_argument(argument, arg);
	if (!*arg) {
		if (GET_INVIS_LEV(ch) > 0)
			perform_immort_vis(ch);
		else {
			if (GetRealLevel(ch) < kLvlImplementator)
				perform_immort_invis(ch, kLvlImmortal);
			else
				perform_immort_invis(ch, GetRealLevel(ch));
		}
	} else {
		level = MIN(atoi(arg), kLvlImplementator);
		if (level > GetRealLevel(ch) && !ch->IsFlagged(EPrf::kCoderinfo))
			SendMsgToChar("Вы не можете достичь невидимости выше вашего уровня.\r\n", ch);
		else if (GetRealLevel(ch) < kLvlImplementator && level > kLvlImmortal && !ch->IsFlagged(EPrf::kCoderinfo))
			perform_immort_invis(ch, kLvlImmortal);
		else if (level < 1)
			perform_immort_vis(ch);
		else
			perform_immort_invis(ch, level);
	}
}

void do_gecho(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	DescriptorData *pt;

	skip_spaces(&argument);
	delete_doubledollar(argument);

	if (!*argument) {
		SendMsgToChar("Это, пожалуй, ошибка...\r\n", ch);
	} else {
		sprintf(buf, "%s\r\n", argument);
		for (pt = descriptor_list; pt; pt = pt->next) {
			if (STATE(pt) == CON_PLAYING
				&& pt->character
				&& pt->character.get() != ch) {
				SendMsgToChar(buf, pt->character.get());
			}
		}

		if (ch->IsFlagged(EPrf::kNoRepeat)) {
			SendMsgToChar(OK, ch);
		} else {
			SendMsgToChar(buf, ch);
		}
	}
}

void do_poofset(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	char **msg;

	switch (subcmd) {
		case SCMD_POOFIN: msg = &(POOFIN(ch));
			break;
		case SCMD_POOFOUT: msg = &(POOFOUT(ch));
			break;
		default: return;
	}

	skip_spaces(&argument);

	if (*msg)
		free(*msg);

	if (!*argument)
		*msg = nullptr;
	else
		*msg = str_dup(argument);

	SendMsgToChar(OK, ch);
}

void do_dc(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	DescriptorData *d;
	int num_to_dc;
	one_argument(argument, arg);
	if (!(num_to_dc = atoi(arg))) {
		SendMsgToChar("Usage: DC <user number> (type USERS for a list)\r\n", ch);
		return;
	}
	for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

	if (!d) {
		SendMsgToChar("Нет такого соединения.\r\n", ch);
		return;
	}

	if (d->character) //Чтоб не крешило при попытке разъединить незалогиненного
	{
		int victim_level = d->character->IsFlagged(EPrf::kCoderinfo) ? kLvlImplementator : GetRealLevel(d->character);
		int god_level = ch->IsFlagged(EPrf::kCoderinfo) ? kLvlImplementator : GetRealLevel(ch);
		if (victim_level >= god_level) {
			if (!CAN_SEE(ch, d->character))
				SendMsgToChar("Нет такого соединения.\r\n", ch);
			else
				SendMsgToChar("Да уж.. Это не есть праффильная идея...\r\n", ch);
			return;
		}
	}

	/* We used to just close the socket here using close_socket(), but
	 * various people pointed out this could cause a crash if you're
	 * closing the person below you on the descriptor list.  Just setting
	 * to CON_CLOSE leaves things in a massively inconsistent state so I
	 * had to add this new flag to the descriptor.
	 *
	 * It is a much more logical extension for a CON_DISCONNECT to be used
	 * for in-game socket closes and CON_CLOSE for out of game closings.
	 * This will retain the stability of the close_me hack while being
	 * neater in appearance. -gg 12/1/97
	 */
	if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
		SendMsgToChar("Соединение уже разорвано.\r\n", ch);
	else {
		/*
		 * Remember that we can disconnect people not in the game and
		 * that rather confuses the code when it expected there to be
		 * a character context.
		 */
		if (STATE(d) == CON_PLAYING)
			STATE(d) = CON_DISCONNECT;
		else
			STATE(d) = CON_CLOSE;

		sprintf(buf, "Соединение #%d закрыто.\r\n", num_to_dc);
		SendMsgToChar(buf, ch);
		imm_log("Connect closed by %s.", GET_NAME(ch));
	}
}

void do_wizlock(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int value;
	const char *when;

	one_argument(argument, arg);
	if (*arg) {
		value = atoi(arg);
		if (value > kLvlImplementator)
			value = kLvlImplementator; // 34е всегда должны иметь возможность зайти
		if (value < 0 || (value > GetRealLevel(ch) && !ch->IsFlagged(EPrf::kCoderinfo))) {
			SendMsgToChar("Неверное значение для wizlock.\r\n", ch);
			return;
		}
		circle_restrict = value;
		when = "теперь";
	} else
		when = "в настоящее время";

	switch (circle_restrict) {
		case 0: sprintf(buf, "Игра %s полностью открыта.\r\n", when);
			break;
		case 1: sprintf(buf, "Игра %s закрыта для новых игроков.\r\n", when);
			break;
		default:
			sprintf(buf, "Только игроки %d %s и выше могут %s войти в игру.\r\n",
					circle_restrict, GetDeclensionInNumber(circle_restrict, EWhat::kLvl), when);
			break;
	}
	SendMsgToChar(buf, ch);
}

extern void PrintUptime(std::ostringstream &out);

void do_date(CharData *ch, char * /*argument*/, int/* cmd*/, int subcmd) {
	time_t mytime;
	std::ostringstream out;

	if (subcmd == SCMD_DATE) {
		mytime = time(nullptr);
		out << "Текущее время сервера: " << asctime(localtime(&mytime)) << "\r\n";
	} else {
		mytime = shutdown_parameters.get_boot_time();
		out << " Up since: " << asctime(localtime(&mytime));
		out.seekp(-1, std::ios_base::end); // Удаляем \0 из конца строки.
		out << " ";
		PrintUptime(out);
	}

	SendMsgToChar(out.str(), ch);
}

void do_last(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	one_argument(argument, arg);
	if (!*arg) {
		SendMsgToChar("Кого вы хотите найти?\r\n", ch);
		return;
	}

	Player t_chdata;
	Player *chdata = &t_chdata;
	if (LoadPlayerCharacter(arg, chdata, ELoadCharFlags::kFindId) < 0) {
		SendMsgToChar("Нет такого игрока.\r\n", ch);
		return;
	}
	if (GetRealLevel(chdata) > GetRealLevel(ch) && !IS_IMPL(ch) && !ch->IsFlagged(EPrf::kCoderinfo)) {
		SendMsgToChar("Вы не столь уж и божественны для этого.\r\n", ch);
	} else {
		time_t tmp_time = LAST_LOGON(chdata);
		sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
				GET_IDNUM(chdata), GetRealLevel(chdata),
				MUD::Class(chdata->GetClass()).GetAbbr().c_str(), GET_NAME(chdata),
				GET_LASTIP(chdata)[0] ? GET_LASTIP(chdata) : "Unknown", ctime(&tmp_time));
		SendMsgToChar(buf, ch);
	}
}

void do_force(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	DescriptorData *i, *next_desc;
	char to_force[kMaxInputLength + 2];

	half_chop(argument, arg, to_force);

	sprintf(buf1, "$n принудил$g вас '%s'.", to_force);

	if (!*arg || !*to_force) {
		SendMsgToChar("Кого и что вы хотите принудить сделать?\r\n", ch);
	} else if (!IS_GRGOD(ch) || (str_cmp("all", arg) && str_cmp("room", arg) && str_cmp("все", arg)
		&& str_cmp("здесь", arg))) {
		const auto vict = get_char_vis(ch, arg, EFind::kCharInWorld);
		if (!vict) {
			SendMsgToChar(NOPERSON, ch);
		} else if (!vict->IsNpc() && GetRealLevel(ch) <= GetRealLevel(vict) && !ch->IsFlagged(EPrf::kCoderinfo)) {
			SendMsgToChar("Господи, только не это!\r\n", ch);
		} else {
			char *pstr;
			SendMsgToChar(OK, ch);
			act(buf1, true, ch, nullptr, vict, kToVict);
			sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
			while ((pstr = strchr(buf, '%')) != nullptr) {
				pstr[0] = '*';
			}
			mudlog(buf, NRM, std::max(kLvlGod, GET_INVIS_LEV(ch)), SYSLOG, true);
			imm_log("%s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
			command_interpreter(vict, to_force);
		}
	} else if (!str_cmp("room", arg)
		|| !str_cmp("здесь", arg)) {
		SendMsgToChar(OK, ch);
		sprintf(buf, "(GC) %s forced room %d to %s", GET_NAME(ch), GET_ROOM_VNUM(ch->in_room), to_force);
		mudlog(buf, NRM, std::max(kLvlGod, GET_INVIS_LEV(ch)), SYSLOG, true);
		imm_log("%s forced room %d to %s", GET_NAME(ch), GET_ROOM_VNUM(ch->in_room), to_force);

		const auto people_copy = world[ch->in_room]->people;
		for (const auto vict : people_copy) {
			if (!vict->IsNpc()
				&& GetRealLevel(vict) >= GetRealLevel(ch)
				&& !ch->IsFlagged(EPrf::kCoderinfo)) {
				continue;
			}

			act(buf1, true, ch, nullptr, vict, kToVict);
			command_interpreter(vict, to_force);
		}
	} else        // force all
	{
		SendMsgToChar(OK, ch);
		sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
		mudlog(buf, NRM, std::max(kLvlGod, GET_INVIS_LEV(ch)), SYSLOG, true);
		imm_log("%s forced all to %s", GET_NAME(ch), to_force);

		for (i = descriptor_list; i; i = next_desc) {
			next_desc = i->next;

			const auto vict = i->character;
			if (STATE(i) != CON_PLAYING
				|| !vict
				|| (!vict->IsNpc() && GetRealLevel(vict) >= GetRealLevel(ch)
					&& !ch->IsFlagged(EPrf::kCoderinfo))) {
				continue;
			}

			act(buf1, true, ch, nullptr, vict.get(), kToVict);
			command_interpreter(vict.get(), to_force);
		}
	}
}

void do_sdemigod(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	DescriptorData *d;
	// убираем пробелы
	skip_spaces(&argument);

	if (!*argument) {
		SendMsgToChar("Что Вы хотите сообщить ?\r\n", ch);
		return;
	}
	sprintf(buf1, "&c%s демигодам: '%s'&n\r\n", GET_NAME(ch), argument);

	// проходим по всем чарам и отправляем мессагу
	for (d = descriptor_list; d; d = d->next) {
		// Проверяем, в игре ли чар
		if (STATE(d) == CON_PLAYING) {
			// Если в игре, то проверяем, демигод ли чар
			// Иммы 34-левела тоже могут смотреть канал
			if ((GET_GOD_FLAG(d->character, EGf::kDemigod)) || (GetRealLevel(d->character) == kLvlImplementator)) {
				// Проверяем пишет ли чар или отправляет письмо
				// А так же на реж сдемигод
				if ((!d->character->IsFlagged(EPlrFlag::kWriting)) &&
					(!d->character->IsFlagged(EPlrFlag::kMailing)) &&
					(!d->character->IsFlagged(EPrf::kDemigodChat))) {
					d->character->remember_add(buf1, Remember::ALL);
					SendMsgToChar(buf1, d->character.get());
				}
			}
		}
	}
}

void do_wiznet(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	DescriptorData *d;
	char emote = false;
	char bookmark1 = false;
	char bookmark2 = false;
	int level = kLvlGod;

	skip_spaces(&argument);
	delete_doubledollar(argument);

	if (!*argument) {
		SendMsgToChar
			("Формат: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
			 "        wiznet @<level> *<emotetext> | wiz @\r\n", ch);
		return;
	}

	//	if (EPrf::FLAGGED(ch, EPrf::CODERINFO)) return;

	// Опускаем level для gf_demigod
	if (GET_GOD_FLAG(ch, EGf::kDemigod))
		level = kLvlImmortal;

	// использование доп. аргументов
	switch (*argument) {
		case '*': emote = true;
			break;
		case '#':
			// Установить уровень имм канала
			one_argument(argument + 1, buf1);
			if (is_number(buf1)) {
				half_chop(argument + 1, buf1, argument);
				level = MAX(atoi(buf1), kLvlImmortal);
				if (level > GetRealLevel(ch)) {
					SendMsgToChar("Вы не можете изрекать выше вашего уровня.\r\n", ch);
					return;
				}
			} else if (emote)
				argument++;
			break;
		case '@':
			// Обнаруживаем всех кто может (теоретически) нас услышать
			for (d = descriptor_list; d; d = d->next) {
				if (STATE(d) == CON_PLAYING &&
					(IS_IMMORTAL(d->character) || GET_GOD_FLAG(d->character, EGf::kDemigod)) &&
					!d->character->IsFlagged(EPrf::kNoWiz) && (CAN_SEE(ch, d->character) || IS_IMPL(ch))) {
					if (!bookmark1) {
						strcpy(buf1,
							   "Боги/привилегированные которые смогут (наверное) вас услышать:\r\n");
						bookmark1 = true;
					}
					sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
					if (d->character->IsFlagged(EPlrFlag::kWriting))
						strcat(buf1, " (пишет)\r\n");
					else if (d->character->IsFlagged(EPlrFlag::kMailing))
						strcat(buf1, " (пишет письмо)\r\n");
					else
						strcat(buf1, "\r\n");
				}
			}
			for (d = descriptor_list; d; d = d->next) {
				if (STATE(d) == CON_PLAYING &&
					(IS_IMMORTAL(d->character) || GET_GOD_FLAG(d->character, EGf::kDemigod)) &&
					d->character->IsFlagged(EPrf::kNoWiz) && CAN_SEE(ch, d->character)) {
					if (!bookmark2) {
						if (!bookmark1)
							strcpy(buf1,
								   "Боги/привилегированные которые не смогут вас услышать:\r\n");
						else
							strcat(buf1,
								   "Боги/привилегированные которые не смогут вас услышать:\r\n");

						bookmark2 = true;
					}
					sprintf(buf1 + strlen(buf1), "  %s\r\n", GET_NAME(d->character));
				}
			}
			SendMsgToChar(buf1, ch);

			return;
		default: break;
	}
	if (ch->IsFlagged(EPrf::kNoWiz)) {
		SendMsgToChar("Вы вне игры!\r\n", ch);
		return;
	}
	skip_spaces(&argument);

	if (!*argument) {
		SendMsgToChar("Не думаю, что Боги одобрят это.\r\n", ch);
		return;
	}
	if (level != kLvlGod) {
		sprintf(buf1, "%s%s: <%d> %s%s\r\n", GET_NAME(ch),
				emote ? "" : " богам", level, emote ? "<--- " : "", argument);
	} else {
		sprintf(buf1, "%s%s: %s%s\r\n", GET_NAME(ch), emote ? "" : " богам", emote ? "<--- " : "", argument);
	}
	snprintf(buf2, kMaxStringLength, "&c%s&n", buf1);
	Remember::add_to_flaged_cont(Remember::wiznet_, buf2, level);

	// пробегаемся по списку дескрипторов чаров и кто должен - тот услышит богов
	for (d = descriptor_list; d; d = d->next) {
		if ((STATE(d) == CON_PLAYING) &&    // персонаж должен быть в игре
			((GetRealLevel(d->character) >= level) ||    // уровень равным или выше level
				(GET_GOD_FLAG(d->character, EGf::kDemigod) && level == 31)) &&    // демигоды видят 31 канал
			(!d->character->IsFlagged(EPrf::kNoWiz)) &&    // игрок с режимом NOWIZ не видит имм канала
			(!d->character->IsFlagged(EPlrFlag::kWriting)) &&    // пишущий не видит имм канала
			(!d->character->IsFlagged(EPlrFlag::kMailing)))    // отправляющий письмо не видит имм канала
		{
			// отправляем сообщение чару
			snprintf(buf2, kMaxStringLength, "%s%s%s",
					 CCCYN(d->character, C_NRM), buf1, CCNRM(d->character, C_NRM));
			d->character->remember_add(buf2, Remember::ALL);
			// не видино своих мессаг если 'режим repeat'
			if (d != ch->desc
				|| !(d->character->IsFlagged(EPrf::kNoRepeat))) {
				SendMsgToChar(buf2, d->character.get());
			}
		}
	}
	if (ch->IsFlagged(EPrf::kNoRepeat)) {
		SendMsgToChar(OK, ch);
	}
}

void do_zreset(CharData *ch, char *argument, int cmd, int/* subcmd*/) {
	ZoneRnum i;
	ZoneVnum j;
	std::vector<ZoneRnum> zone_repop_list;
	one_argument(argument, arg);

	if (!(privilege::HasPrivilege(ch, std::string(cmd_info[cmd].command), 0, 0, false)) && (GET_OLC_ZONE(ch) <= 0)) {
		SendMsgToChar("Чаво?\r\n", ch);
		return;
	}

	if (!*arg) {
		SendMsgToChar("Укажите зону.\r\n", ch);
		return;
	}
	int first = atoi(arg);

	if (!IS_IMMORTAL(ch) && GET_OLC_ZONE(ch) != first) {
		SendMsgToChar("Доступ к данной зоне запрещен!\r\n", ch);
		return;
	}

	if (*arg == '*') {
		for (i = 0; i < static_cast<ZoneRnum>(zone_table.size()); i++) {
			zone_repop_list.push_back(i);
		}
		DecayObjectsOnRepop(zone_repop_list);
		for (i = 0; i < static_cast<ZoneRnum>(zone_table.size()); i++) {
			ResetZone(i);
		}
		SendMsgToChar("Перезагружаю мир.\r\n", ch);
		sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
		mudlog(buf, NRM, MAX(kLvlGreatGod, GET_INVIS_LEV(ch)), SYSLOG, true);
		imm_log("%s reset entire world.", GET_NAME(ch));
		return;
	} else if (*arg == '.') {
		i = world[ch->in_room]->zone_rn;
	} else {
		j = atoi(arg);
		for (i = 0; i < static_cast<ZoneRnum>(zone_table.size()); i++) {
			if (zone_table[i].vnum == j) {
				break;
			}
		}
	}

	if (i >= 0 && i < static_cast<ZoneRnum>(zone_table.size())) {
		zone_repop_list.push_back(i);
		DecayObjectsOnRepop(zone_repop_list);
		utils::CExecutionTimer timer;
		ResetZone(i);
		sprintf(buf, "Перегружаю зону %d (#%d): %s, delta %f\r\n", i, zone_table[i].vnum, zone_table[i].name.c_str(), timer.delta().count());
		SendMsgToChar(buf, ch);
		sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name.c_str());
		mudlog(buf, NRM, MAX(kLvlGreatGod, GET_INVIS_LEV(ch)), SYSLOG, true);
		imm_log("%s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name.c_str());
	} else {
		SendMsgToChar("Нет такой зоны.\r\n", ch);
	}
}

// Функции установки разных наказаний.

// *  General fn for wizcommands of the sort: cmd <player>
void do_wizutil(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	CharData *vict;
	long result;
	int times = 0;
	char *reason;
	char num[kMaxInputLength];

	//  one_argument(argument, arg);
	reason = two_arguments(argument, arg, num);

	if (!*arg)
		SendMsgToChar("Для кого?\r\n", ch);
	else if (!(vict = get_player_pun(ch, arg, EFind::kCharInWorld)))
		SendMsgToChar("Нет такого игрока.\r\n", ch);
	else if (GetRealLevel(vict) > GetRealLevel(ch) && !GET_GOD_FLAG(ch, EGf::kDemigod) && !ch->IsFlagged(EPrf::kCoderinfo))
		SendMsgToChar("А он ведь старше вас....\r\n", ch);
	else if (GetRealLevel(vict) >= kLvlImmortal && GET_GOD_FLAG(ch, EGf::kDemigod))
		SendMsgToChar("А он ведь старше вас....\r\n", ch);
	else {
		switch (subcmd) {
			case SCMD_REROLL: SendMsgToChar("Сбрасываю параметры...\r\n", ch);
				vict->set_start_stat(G_STR, 0);
				SendMsgToChar(vict, "&GВам сбросили парамерты персонажа, стоит перезайти в игру.\r\n&n");
/*				roll_real_abils(vict);
				log("(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
				imm_log("%s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
				sprintf(buf,
						"Новые параметры: Str %d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
						vict->GetInbornStr(), vict->GetInbornInt(), vict->GetInbornWis(),
						vict->GetInbornDex(), vict->GetInbornCon(), vict->GetInbornCha());
				SendMsgToChar(buf, ch);
*/
				break;
			case SCMD_NOTITLE:
				vict->IsFlagged(EPlrFlag::kNoTitle) ? vict->UnsetFlag(EPlrFlag::kNoTitle) : vict->SetFlag(EPlrFlag::kNoTitle);
				result = vict->IsFlagged(EPlrFlag::kNoTitle);
				sprintf(buf, "(GC) Notitle %s for %s by %s.", (result ? "ON" : "OFF"), GET_NAME(vict), GET_NAME(ch));
				mudlog(buf, NRM, MAX(kLvlGod, GET_INVIS_LEV(ch)), SYSLOG, true);
				imm_log("Notitle %s for %s by %s.", (result ? "ON" : "OFF"), GET_NAME(vict), GET_NAME(ch));
				strcat(buf, "\r\n");
				SendMsgToChar(buf, ch);
				break;
			case SCMD_SQUELCH: break;
			case SCMD_MUTE: if (*num) times = atol(num);
				set_punish(ch, vict, SCMD_MUTE, reason, times);
				break;
			case SCMD_DUMB: if (*num) times = atol(num);
				set_punish(ch, vict, SCMD_DUMB, reason, times);
				break;
			case SCMD_FREEZE: if (*num) times = atol(num);
				set_punish(ch, vict, SCMD_FREEZE, reason, times);
				break;
			case SCMD_HELL: if (*num) times = atol(num);
				set_punish(ch, vict, SCMD_HELL, reason, times);
				break;

			case SCMD_NAME: if (*num) times = atol(num);
				set_punish(ch, vict, SCMD_NAME, reason, times);
				break;

			case SCMD_REGISTER: set_punish(ch, vict, SCMD_REGISTER, reason, 0);
				break;

			case SCMD_UNREGISTER: set_punish(ch, vict, SCMD_UNREGISTER, reason, 0);
				break;

			case SCMD_UNAFFECT:
				if (!vict->affected.empty()) {
					vict->affected.clear();
					affect_total(vict);
					SendMsgToChar("Яркая вспышка осветила вас!\r\n"
								  "Вы почувствовали себя немного иначе.\r\n", vict);
					SendMsgToChar("Все афекты сняты.\r\n", ch);
				} else {
					SendMsgToChar("Аффектов не было изначально.\r\n", ch);
					return;
				}
				break;
			default: log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
				break;
		}
		vict->save_char();
	}
}

void show_apply(CharData *ch, CharData *vict) {
	ObjData *obj = nullptr;
	for (int i = 0; i < EEquipPos::kNumEquipPos; i++) {
		if ((obj = GET_EQ(vict, i))) {
			SendMsgToChar(ch, "Предмет: %s (%d)\r\n", GET_OBJ_PNAME(obj, 0).c_str(), GET_OBJ_VNUM(obj));
			// Update weapon applies
			for (int j = 0; j < kMaxObjAffect; j++) {
				if (GET_EQ(vict, i)->get_affected(j).modifier != 0) {
						SendMsgToChar(ch, "Добавляет (apply): %s, модификатор: %d\r\n",
							apply_types[(int) GET_EQ(vict, i)->get_affected(j).location], GET_EQ(vict, i)->get_affected(j).modifier);
				}
			}
		}
	}
}

namespace Mlist {

std::string print_race(CharData *mob) {
	std::string out;
	if (GET_RACE(mob) <= ENpcRace::kLastNpcRace) {
		out += npc_race_types[GET_RACE(mob) - ENpcRace::kBasic];
	} else {
		out += "UNDEF";
	}
	return out;
}

std::string print_role(CharData *mob) {
	std::string out;
	if (mob->get_role_bits().any()) {
		print_bitset(mob->get_role_bits(), npc_role_types, ",", out);
	} else {
		out += "---";
	}
	return out;
}

std::string print_flag(CharData *ch, CharData *mob, const std::string &options) {
	std::vector<std::string> option_list = utils::SplitAny(options, ",. ");
	auto out = fmt::memory_buffer();

	for (const auto & i : option_list) {
		if (isname(i, "race")) {
			format_to(std::back_inserter(out), " [раса: {}{}{} ]",
					CCCYN(ch, C_NRM), print_race(mob), CCNRM(ch, C_NRM));
		}
		if (isname(i, "role")) {
			format_to(std::back_inserter(out), " [роли: {}{}{} ]",
					CCCYN(ch, C_NRM), print_role(mob), CCNRM(ch, C_NRM));
		} 
		format_to(std::back_inserter(out), " [спец-проц: {}{}{} ]",
				CCCYN(ch, C_NRM), print_special(mob), CCNRM(ch, C_NRM));
	}
	return to_string(out);
}

void print(CharData *ch, int first, int last, const std::string &options) {
	auto out = fmt::memory_buffer();

	format_to(std::back_inserter(out), "Список мобов от {} до {}\r\n", first, last);
	int cnt = 0;
	for (int i = 0; i <= top_of_mobt; ++i) {
		if (mob_index[i].vnum >= first && mob_index[i].vnum <= last) {
			format_to(std::back_inserter(out), "{:5}. {:<45} [{:<6}] [{:<2}]{}",
					  ++cnt, mob_proto[i].get_name_str().substr(0, 45),
					  mob_index[i].vnum, mob_proto[i].GetLevel(),
					  print_flag(ch, mob_proto + i, options));
			if (!mob_proto[i].proto_script->empty()) {
				format_to(std::back_inserter(out), " - есть скрипты -");
				for (const auto trigger_vnum : *mob_proto[i].proto_script) {
					format_to(std::back_inserter(out), " {}", trigger_vnum);
				}
			} else {
				format_to(std::back_inserter(out), " - нет скриптов -");
			}
			format_to(std::back_inserter(out), " Загружено в мир: {}, максимально: {}.\r\n", mob_index[i].total_online, mob_index[i].total_online);
		}
	}
	if (cnt == 0) {
		SendMsgToChar("Нет мобов в этом промежутке.\r\n", ch);
	} else {
		page_string(ch->desc, to_string(out));
	}
}

} // namespace Mlist

int print_olist(const CharData *ch, const int first, const int last, std::string &out) {
	int result = 0;

	char buf_[256] = {0};
	std::stringstream ss;
	snprintf(buf_, sizeof(buf_), "Список объектов Vnum %d до %d\r\n", first, last);
	ss << buf_;

	auto from = obj_proto.vnum2index().lower_bound(first);
	auto to = obj_proto.vnum2index().upper_bound(last);
	for (auto i = from; i != to; ++i) {
		const auto vnum = i->first;
		const auto rnum = i->second;
		const auto prototype = obj_proto[rnum];

		ss << fmt::format("{:>5}. {} [{:>5}] [ilvl ={} : mort ={}]", ++result,
				colored_name(prototype->get_short_description().c_str(), 45), vnum, prototype->get_ilevel(), prototype->get_auto_mort_req());

		if (GetRealLevel(ch) >= kLvlGreatGod
			|| ch->IsFlagged(EPrf::kCoderinfo)) {
			ss << fmt::format(" Игра:{} Пост:{} Макс:{}", obj_proto.total_online(rnum), obj_proto.stored(rnum), GetObjMIW(rnum));
			const auto &script = prototype->get_proto_script();

			if (!script.empty()) {
				ss << " - есть скрипты -";
				for (const auto trigger_vnum : script) {
					ss << fmt::format(" [{}]", trigger_vnum);
				}
			} else {
				ss << " - нет скриптов";
			}
		}
		ss << "\r\n";
	}

	out = ss.str();
	return result;
}

void do_liblist(CharData *ch, char *argument, int cmd, int subcmd) {
	int first, last, nr, found = 0;

	argument = two_arguments(argument, buf, buf2);
	first = atoi(buf);
	if (!(privilege::HasPrivilege(ch, std::string(cmd_info[cmd].command), 0, 0, false)) && (GET_OLC_ZONE(ch) != first)) {
		SendMsgToChar("Чаво?\r\n", ch);
		return;
	}
	if (!*buf || (!*buf2 && (subcmd == SCMD_ZLIST))) {
		switch (subcmd) {
			case SCMD_RLIST:
				SendMsgToChar("Использование: ксписок <начальный номер или номер зоны> [<конечный номер>]\r\n",
							  ch);
				break;
			case SCMD_OLIST:
				SendMsgToChar("Использование: осписок <начальный номер или номер зоны> [<конечный номер>]\r\n",
							  ch);
				break;
			case SCMD_MLIST:
				SendMsgToChar("Использование: мсписок <начальный номер или номер зоны> [<конечный номер>] [role race]\r\n",
							  ch);
				break;
			case SCMD_ZLIST: SendMsgToChar("Использование: зсписок <начальный номер> <конечный номер>\r\n", ch);
				break;
			case SCMD_CLIST:
				SendMsgToChar("Использование: ксписок <начальный номер или номер зоны> [<конечный номер>]\r\n",
							  ch);
				break;
			default: sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
				mudlog(buf, BRF, kLvlGod, SYSLOG, true);
				break;
		}
		return;
	}

	if (*buf2 && a_isdigit(buf2[0])) {
		last = atoi(buf2);
	} else {
		first *= 100;
		last = first + 99;
	}

	if ((first < 0) || (first > kMaxProtoNumber) || (last < 0) || (last > kMaxProtoNumber)) {
		sprintf(buf, "Значения должны быть между 0 и %d.\n\r", kMaxProtoNumber);
		SendMsgToChar(buf, ch);
		return;
	}

	if (first > last) {
		std::swap(first, last);
	}

	if (first + 100 < last) {
		SendMsgToChar("Максимальный показываемый промежуток - 100.\n\r", ch);
		return;
	}

	char buf_[256];
	std::string out;

	switch (subcmd) {
		case SCMD_RLIST:
			snprintf(buf_, sizeof(buf_),
					 "Список комнат от Vnum %d до %d\r\n", first, last);
			out += buf_;
			for (nr = kFirstRoom; nr <= top_of_world && (world[nr]->vnum <= last); nr++) {
				if (world[nr]->vnum >= first) {
					snprintf(buf_, sizeof(buf_), "%5d. [%5d] (%3d) %s",
							 ++found, world[nr]->vnum, nr, world[nr]->name);
					out += buf_;
					if (!world[nr]->proto_script->empty()) {
						out += " - есть скрипты -";
						for (const auto trigger_vnum : *world[nr]->proto_script) {
							sprintf(buf1, " [%d]", trigger_vnum);
							out += buf1;
						}
						out += "\r\n";
					} else {
						out += " - нет скриптов\r\n";
					}
				}
			}
			break;
		case SCMD_OLIST: found = print_olist(ch, first, last, out);
			break;

		case SCMD_MLIST: {
			std::string option;
			if (*buf2 && !a_isdigit(buf2[0])) {
				option = buf2;
			}
			option += " ";
			option += argument;
			Mlist::print(ch, first, last, option);
			return;
		}
		case SCMD_ZLIST:
			snprintf(buf_, sizeof(buf_),
					 "Список зон от %d до %d\r\n"
					 "(флаги, номер, резет, уровень/средний уровень мобов, группа, имя)\r\n",
					 first, last);
			out += buf_;

			for (nr = 0; nr < static_cast<ZoneRnum>(zone_table.size()) && (zone_table[nr].vnum <= last); nr++) {
				if (zone_table[nr].vnum >= first) {
					snprintf(buf_, sizeof(buf_),
							 "%5d. [%s%s] [%5d] (%3d) (%2d/%2d) (%2d) %s\r\n",
							 ++found,
							 zone_table[nr].locked ? "L" : " ",
							 zone_table[nr].under_construction ? "T" : " ",
							 zone_table[nr].vnum,
							 zone_table[nr].lifespan,
							 zone_table[nr].level,
							 zone_table[nr].mob_level,
							 zone_table[nr].group,
							 zone_table[nr].name.c_str());
					out += buf_;
				}
			}
			break;

		case SCMD_CLIST: out = "Заглушка. Возможно, будет использоваться в будущем\r\n";
			break;

		default: sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
			mudlog(buf, BRF, kLvlGod, SYSLOG, true);
			return;
	}

	if (!found) {
		switch (subcmd) {
			case SCMD_RLIST: SendMsgToChar("Нет комнат в этом промежутке.\r\n", ch);
				break;
			case SCMD_OLIST: SendMsgToChar("Нет объектов в этом промежутке.\r\n", ch);
				break;
			case SCMD_ZLIST: SendMsgToChar("Нет зон в этом промежутке.\r\n", ch);
				break;
			default: sprintf(buf, "SYSERR:: invalid SCMD passed to do_build_list!");
				mudlog(buf, BRF, kLvlGod, SYSLOG, true);
				break;
		}
		return;
	}

	page_string(ch->desc, out);
}

void do_forcetime(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	int m, t = 0;
	char *ca;

	// Parse command line
	for (ca = strtok(argument, " "); ca; ca = strtok(nullptr, " ")) {
		m = LOWER(ca[strlen(ca) - 1]);
		if (m == 'h')    // hours
			m = 60 * 60;
		else if (m == 'm')    // minutes
			m = 60;
		else if (m == 's')    // seconds
			m = 1;
		else
			m = 0;

		if ((m *= atoi(ca)) > 0)
			t += m;
		else {
			// no time shift with undefined arguments
			t = 0;
			break;
		}
	}

	if (t <= 0) {
		SendMsgToChar("Сдвиг игрового времени (h - часы, m - минуты, s - секунды).\r\n", ch);
			return;
	}

	for (m = 0; m < t * kPassesPerSec; m++) {
		GlobalObjects::heartbeat()(t * kPassesPerSec - m);
	}

	SendMsgToChar(ch, "Вы перевели игровое время на %d сек вперед.\r\n", t);
	sprintf(buf, "(GC) %s перевел игровое время на %d сек вперед.", GET_NAME(ch), t);
	mudlog(buf, NRM, kLvlImmortal, IMLOG, false);
	SendMsgToChar(OK, ch);

}

///////////////////////////////////////////////////////////////////////////////
namespace SpellUsage {
bool is_active = false;
std::map<ECharClass, SpellCountType> usage;
const char *SPELL_STAT_FILE = LIB_STAT"spellstat.txt";
time_t start;
}

void SpellUsage::clear() {
	for (auto & it : usage) {
		it.second.clear();
	}
	usage.clear();
	start = time(nullptr);
}

std::string statToPrint() {
	std::stringstream out;
	time_t now = time(nullptr);
	char *end_time = str_dup(rustime(localtime(&now)));
	out << rustime(localtime(&SpellUsage::start)) << " - " << end_time << "\n";
	for (auto & it : SpellUsage::usage) {
		out << std::setw(35) << MUD::Class(it.first).GetName() << "\r\n";
		for (auto & itt : it.second) {
			out << std::setw(25) << MUD::Spell(itt.first).GetName() << " : " << itt.second << "\r\n";
		}
	}
	return out.str();
}

void SpellUsage::save() {
	if (!is_active)
		return;

	std::ofstream file(SPELL_STAT_FILE, std::ios_base::app | std::ios_base::out);

	if (!file.is_open()) {
		log("Error open file: %s! (%s %s %d)", SPELL_STAT_FILE, __FILE__, __func__, __LINE__);
		return;
	}
	file << statToPrint();
	file.close();
}

void SpellUsage::AddSpellStat(ECharClass char_class, ESpell spell_id) {
	if (!is_active) {
		return;
	}
	if (MUD::Classes().IsUnavailable(char_class) || spell_id > ESpell::kLast) {
		return;
	}
	++usage[char_class][spell_id];
}

void do_spellstat(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	skip_spaces(&argument);

	if (!*argument) {
		SendMsgToChar("заклстат [стоп|старт|очистить|показать|сохранить]\r\n", ch);
		return;
	}

	if (!str_cmp(argument, "старт")) {
		SpellUsage::is_active = true;
		SpellUsage::start = time(nullptr);
		SendMsgToChar("Сбор включен.\r\n", ch);
		return;
	}

	if (!SpellUsage::is_active) {
		SendMsgToChar("Сбор выключен. Включите сбор 'заклстат старт'.\r\n", ch);
		return;
	}

	if (!str_cmp(argument, "стоп")) {
		SpellUsage::clear();
		SpellUsage::is_active = false;
		SendMsgToChar("Сбор выключен.\r\n", ch);
		return;
	}

	if (!str_cmp(argument, "показать")) {
		page_string(ch->desc, statToPrint());
		return;
	}

	if (!str_cmp(argument, "очистить")) {
		SpellUsage::clear();
		return;
	}

	if (!str_cmp(argument, "сохранить")) {
		SpellUsage::save();
		return;
	}

	SendMsgToChar("заклстат: неизвестный аргумент\r\n", ch);
}

void do_sanitize(CharData *ch, char * /*argument*/, int/* cmd*/, int/* subcmd*/) {
	SendMsgToChar("Запущена процедура сбора мусора после праздника...\r\n", ch);
	celebrates::Sanitize();
}

void do_loadstat(CharData *ch, char * /*argument*/, int/* cmd*/, int/* subcmd*/) {
	std::ifstream istream(LOAD_LOG_FOLDER LOAD_LOG_FILE, std::ifstream::in);
	int length;

	if (!istream.is_open()) {
		SendMsgToChar("Can't open file", ch);
		log("ERROR: Can't open file %s", LOAD_LOG_FOLDER LOAD_LOG_FILE);
		return;
	}

	istream.seekg(0, std::ifstream::end);
	length = istream.tellg();
	istream.seekg(0, std::ifstream::beg);
	istream.read(buf, std::min(length, kMaxStringLength - 1));
	buf[istream.gcount()] = '\0';
	SendMsgToChar(buf, ch);
}

namespace {

struct filter_type {
	filter_type() : type(-1), wear(EWearFlag::kUndefined), wear_message(-1), material(-1) {};

	// тип
	int type;
	// куда одевается
	EWearFlag wear;
	// для названия куда одеть
	int wear_message;
	// материал
	int material;
	// аффекты weap
	std::vector<int> affect;
	// аффекты apply
	std::vector<int> affect2;
	// экстрафлаг
	std::vector<int> affect3;
};

} // namespace

void do_print_armor(CharData *ch, char *argument, int/* cmd*/, int/* subcmd*/) {
	if (ch->IsNpc() || (!IS_GRGOD(ch) && !ch->IsFlagged(EPrf::kCoderinfo))) {
		SendMsgToChar("Чаво?\r\n", ch);
		return;
	}

	filter_type filter;
	char tmpbuf[kMaxInputLength];
	bool find_param = false;
	while (*argument) {
		switch (*argument) {
			case 'М': argument = one_argument(++argument, tmpbuf);
				if (utils::IsAbbr(tmpbuf, "булат")) {
					filter.material = EObjMaterial::kBulat;
				} else if (utils::IsAbbr(tmpbuf, "бронза")) {
					filter.material = EObjMaterial::kBronze;
				} else if (utils::IsAbbr(tmpbuf, "железо")) {
					filter.material = EObjMaterial::kIron;
				} else if (utils::IsAbbr(tmpbuf, "сталь")) {
					filter.material = EObjMaterial::kSteel;
				} else if (utils::IsAbbr(tmpbuf, "кованая.сталь")) {
					filter.material = EObjMaterial::kForgedSteel;
				} else if (utils::IsAbbr(tmpbuf, "драг.металл")) {
					filter.material = EObjMaterial::kPreciousMetel;
				} else if (utils::IsAbbr(tmpbuf, "кристалл")) {
					filter.material = EObjMaterial::kCrystal;
				} else if (utils::IsAbbr(tmpbuf, "дерево")) {
					filter.material = EObjMaterial::kWood;
				} else if (utils::IsAbbr(tmpbuf, "прочное.дерево")) {
					filter.material = EObjMaterial::kHardWood;
				} else if (utils::IsAbbr(tmpbuf, "керамика")) {
					filter.material = EObjMaterial::kCeramic;
				} else if (utils::IsAbbr(tmpbuf, "стекло")) {
					filter.material = EObjMaterial::kGlass;
				} else if (utils::IsAbbr(tmpbuf, "камень")) {
					filter.material = EObjMaterial::kStone;
				} else if (utils::IsAbbr(tmpbuf, "кость")) {
					filter.material = EObjMaterial::kBone;
				} else if (utils::IsAbbr(tmpbuf, "ткань")) {
					filter.material = EObjMaterial::kCloth;
				} else if (utils::IsAbbr(tmpbuf, "кожа")) {
					filter.material = EObjMaterial::kSkin;
				} else if (utils::IsAbbr(tmpbuf, "органика")) {
					filter.material = EObjMaterial::kOrganic;
				} else if (utils::IsAbbr(tmpbuf, "береста")) {
					filter.material = EObjMaterial::kPaper;
				} else if (utils::IsAbbr(tmpbuf, "драг.камень")) {
					filter.material = EObjMaterial::kDiamond;
				} else {
					SendMsgToChar("Неверный материал предмета.\r\n", ch);
					return;
				}
				find_param = true;
				break;
			case 'Т': argument = one_argument(++argument, tmpbuf);
				if (utils::IsAbbr(tmpbuf, "броня") || utils::IsAbbr(tmpbuf, "armor")) {
					filter.type = EObjType::kArmor;
				} else if (utils::IsAbbr(tmpbuf, "легкие") || utils::IsAbbr(tmpbuf, "легкая")) {
					filter.type = EObjType::kLightArmor;
				} else if (utils::IsAbbr(tmpbuf, "средние") || utils::IsAbbr(tmpbuf, "средняя")) {
					filter.type = EObjType::kMediumArmor;
				} else if (utils::IsAbbr(tmpbuf, "тяжелые") || utils::IsAbbr(tmpbuf, "тяжелая")) {
					filter.type = EObjType::kHeavyArmor;
				} else {
					SendMsgToChar("Неверный тип предмета.\r\n", ch);
					return;
				}
				find_param = true;
				break;
			case 'О': argument = one_argument(++argument, tmpbuf);
				if (utils::IsAbbr(tmpbuf, "тело")) {
					filter.wear = EWearFlag::kBody;
					filter.wear_message = 3;
				} else if (utils::IsAbbr(tmpbuf, "голова")) {
					filter.wear = EWearFlag::kHead;
					filter.wear_message = 4;
				} else if (utils::IsAbbr(tmpbuf, "ноги")) {
					filter.wear = EWearFlag::kLegs;
					filter.wear_message = 5;
				} else if (utils::IsAbbr(tmpbuf, "ступни")) {
					filter.wear = EWearFlag::kFeet;
					filter.wear_message = 6;
				} else if (utils::IsAbbr(tmpbuf, "кисти")) {
					filter.wear = EWearFlag::kHands;
					filter.wear_message = 7;
				} else if (utils::IsAbbr(tmpbuf, "руки")) {
					filter.wear = EWearFlag::kArms;
					filter.wear_message = 8;
				} else {
					SendMsgToChar("Неверное место одевания предмета.\r\n", ch);
					return;
				}
				find_param = true;
				break;
			case 'А': {
				bool tmp_find = false;
				argument = one_argument(++argument, tmpbuf);
				if (!strlen(tmpbuf)) {
					SendMsgToChar("Неверный аффект предмета.\r\n", ch);
					return;
				}
				if (filter.affect.size() + filter.affect2.size() + filter.affect3.size() >= 3) {
					break;
				}
				switch (*tmpbuf) {
					case '1': sprintf(tmpbuf, "можно вплавить 1 камень");
						break;
					case '2': sprintf(tmpbuf, "можно вплавить 2 камня");
						break;
					case '3': sprintf(tmpbuf, "можно вплавить 3 камня");
						break;
					default: break;
				}
				utils::ConvertToLow(tmpbuf);
				size_t len = strlen(tmpbuf);
				int num = 0;

				for (int flag = 0; flag < 4; ++flag) {
					for (/* тут ничего не надо */; *weapon_affects[num] != '\n'; ++num) {
						if (strlen(weapon_affects[num]) < len)
							continue;
						if (!strncmp(weapon_affects[num], tmpbuf, len)) {
							filter.affect.push_back(num);
							tmp_find = true;
							break;
						}
					}
					if (tmp_find) {
						break;
					}
					++num;
				}
				if (!tmp_find) {
					for (num = 0; *apply_types[num] != '\n'; ++num) {
						if (strlen(apply_types[num]) < len)
							continue;
						if (!strncmp(apply_types[num], tmpbuf, len)) {
							filter.affect2.push_back(num);
							tmp_find = true;
							break;
						}
					}
				}
				// поиск по экстрафлагу
				if (!tmp_find) {
					num = 0;
					for (int flag = 0; flag < 4; ++flag) {
						for (/* тут ничего не надо */; *extra_bits[num] != '\n'; ++num) {
							if (strlen(extra_bits[num]) < len)
								continue;
							if (!strncmp(extra_bits[num], tmpbuf, len)) {
								filter.affect3.push_back(num);
								tmp_find = true;
								break;
							}
						}
						if (tmp_find) {
							break;
						}
						num++;
					}
				}
				if (!tmp_find) {
					sprintf(buf, "Неверный аффект предмета: '%s'.\r\n", tmpbuf);
					SendMsgToChar(buf, ch);
					return;
				}
				find_param = true;
				break;
			}
			default: ++argument;
		}
	}
	if (!find_param) {
		SendMsgToChar("Формат команды:\r\n"
					  "   armor Т[броня|легкие|средние|тяжелые] О[тело|голова|ногиступни|кисти|руки] А[аффект] М[материал]\r\n",
					  ch);
		return;
	}
	std::string buffer = "Выборка по следующим параметрам: ";
	if (filter.material >= 0) {
		buffer += material_name[filter.material];
		buffer += " ";
	}
	if (filter.type >= 0) {
		buffer += item_types[filter.type];
		buffer += " ";
	}
	if (filter.wear != EWearFlag::kUndefined) {
		buffer += wear_bits[filter.wear_message];
		buffer += " ";
	}
	if (!filter.affect.empty()) {
		for (const auto it : filter.affect) {
			buffer += weapon_affects[it];
			buffer += " ";
		}
	}
	if (!filter.affect2.empty()) {
		for (const auto it : filter.affect2) {
			buffer += apply_types[it];
			buffer += " ";
		}
	}
	if (!filter.affect3.empty()) {
		for (const auto it : filter.affect3) {
			buffer += extra_bits[it];
			buffer += " ";
		}
	}
	buffer += "\r\nСредний уровень мобов в зоне | внум предмета  | материал | имя предмета + аффекты если есть\r\n";
	SendMsgToChar(buffer, ch);

	std::multimap<int /* zone lvl */, int /* obj rnum */> tmp_list;
	for (const auto &i : obj_proto) {
		// материал
		if (filter.material >= 0 && filter.material != GET_OBJ_MATER(i)) {
			continue;
		}
		// тип
		if (filter.type >= 0 && filter.type != GET_OBJ_TYPE(i)) {
			continue;
		}
		// куда можно одеть
		if (filter.wear != EWearFlag::kUndefined
			&& !i->has_wear_flag(filter.wear)) {
			continue;
		}
		// аффекты
		bool find = true;
		if (!filter.affect.empty()) {
			for (int it : filter.affect) {
				if (!CompareBits(i->get_affect_flags(), weapon_affects, it)) {
					find = false;
					break;
				}
			}
			// аффект не найден, продолжать смысла нет
			if (!find) {
				continue;
			}
		}

		if (!filter.affect2.empty()) {
			for (auto it = filter.affect2.begin(); it != filter.affect2.end() && find; ++it) {
				find = false;
				for (int k = 0; k < kMaxObjAffect; ++k) {
					if (i->get_affected(k).location == *it) {
						find = true;
						break;
					}
				}
			}
			// доп.свойство не найдено, продолжать смысла нет
			if (!find) {
				continue;
			}
		}
		if (!filter.affect3.empty()) {
			for (auto it = filter.affect3.begin(); it != filter.affect3.end() && find; ++it) {
				//find = true;
				if (!CompareBits(GET_OBJ_EXTRA(i), extra_bits, *it)) {
					find = false;
					break;
				}
			}
			// экстрафлаг не найден, продолжать смысла нет
			if (!find) {
				continue;
			}
		}

		if (find) {
			const auto vnum = i->get_vnum() / 100;
			for (auto & nr : zone_table) {
				if (vnum == nr.vnum) {
					tmp_list.insert(std::make_pair(nr.mob_level, GET_OBJ_RNUM(i)));
				}
			}
		}
	}

	std::ostringstream out;
	for (auto it = tmp_list.rbegin(), iend = tmp_list.rend(); it != iend; ++it) {
		const auto obj = obj_proto[it->second];
		out << "   "
			<< std::setw(2) << it->first << " | "
			<< std::setw(7) << obj->get_vnum() << " | "
			<< std::setw(14) << material_name[GET_OBJ_MATER(obj)] << " | "
			<< GET_OBJ_PNAME(obj, 0) << "\r\n";

		for (int i = 0; i < kMaxObjAffect; i++) {
			auto drndice = obj->get_affected(i).location;
			int drsdice = obj->get_affected(i).modifier;
			if (drndice == EApply::kNone || !drsdice) {
				continue;
			}
			sprinttype(drndice, apply_types, buf2);
			bool negative = IsNegativeApply(drndice);
			if (!negative && drsdice < 0) {
				negative = true;
			} else if (negative && drsdice < 0) {
				negative = false;
			}
			snprintf(buf, kMaxStringLength, "   %s%s%s%s%s%d%s\r\n",
					 CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM),
					 CCCYN(ch, C_NRM),
					 negative ? " ухудшает на " : " улучшает на ", abs(drsdice), CCNRM(ch, C_NRM));
			out << "      |         |                | " << buf;
		}
	}
	if (!out.str().empty()) {
		SendMsgToChar(ch, "Всего найдено предметов: %lu\r\n\r\n", tmp_list.size());
		page_string(ch->desc, out.str());
	} else {
		SendMsgToChar("Ничего не найдено.\r\n", ch);
	}
}

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :
