# -*- coding: koi8-r -*-
import mud

# ���� �������
command_text = "����"
# �������, ������� � ������� ������� ����� ���������. constants.POS_XXX
position_min = 0
# ���. ������� ������ ��� ���������� �������
level_min = 0
# ����������� ������������
unhide_percent = 100
def command(ch, cmd, args):
	""" ������ �������� - ���, ������ �������, ������ ���������"""
	s = "���������: {0}, �������: {1}, ��� ����: {2}".format(args.decode("koi8-r"), cmd.decode("koi8-r"), ch.get_pad(0).decode("koi8-r"))
	ch.send(s.encode("koi8-r"))
