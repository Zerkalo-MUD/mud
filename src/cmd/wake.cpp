//
// Created by Sventovit on 08.09.2024.
//

#include "cmd/wake.h"
#include "entities/char_data.h"
#include "handler.h"

void do_wake(CharData *ch, char *argument, int/* cmd*/, int subcmd) {
	CharData *vict;
	int self = 0;

	one_argument(argument, arg);

	if (subcmd == kScmdWakeUp) {
		if (!(*arg)) {
			SendMsgToChar("Кого будить то будем???\r\n", ch);
			return;
		}
	} else {
		*arg = 0;
	}

	if (*arg) {
		if (GET_POS(ch) == EPosition::kSleep)
			SendMsgToChar("Может быть вам лучше проснуться?\r\n", ch);
		else if ((vict = get_char_vis(ch, arg, EFind::kCharInRoom)) == nullptr)
			SendMsgToChar(NOPERSON, ch);
		else if (vict == ch)
			self = 1;
		else if (AWAKE(vict))
			act("$E и не спал$G.", false, ch, nullptr, vict, kToChar);
		else if (GET_POS(vict) < EPosition::kSleep)
			act("$M так плохо! Оставьте $S в покое!", false, ch, nullptr, vict, kToChar);
		else {
			act("Вы $S разбудили.", false, ch, nullptr, vict, kToChar);
			act("$n растолкал$g вас.", false, ch, nullptr, vict, kToVict | kToSleep);
			GET_POS(vict) = EPosition::kSit;
		}
		if (!self)
			return;
	}
	if (AFF_FLAGGED(ch, EAffect::kSleep))
		SendMsgToChar("Вы не можете проснуться!\r\n", ch);
	else if (GET_POS(ch) > EPosition::kSleep)
		SendMsgToChar("А вы и не спали...\r\n", ch);
	else {
		SendMsgToChar("Вы проснулись и сели.\r\n", ch);
		act("$n проснул$u.", true, ch, nullptr, nullptr, kToRoom | kToArenaListen);
		GET_POS(ch) = EPosition::kSit;
	}
}

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :