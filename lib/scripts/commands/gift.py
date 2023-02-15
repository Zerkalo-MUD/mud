# -*- coding: koi8-r -*-
import mud, os, random, sys
from utils import load_obj



# ���� �������
command_text = "�������"
# �������, ������� � ������� ������� ����� ���������. constants.POS_XXX
position_min = 3
# ���. ������� ������ ��� ���������� �������
level_min = 0
# ����������� ������������
unhide_percent = 100
# ������, � ������� ����� ������� ����
array_email = []
# ������, � ������� �������� ����� ��������
array_vnum = []
def command(ch, cmd, args):
	""" ������ �������� - ���, ������ �������, ������ ���������"""
	global array_email, array_vnum
	email = ch.email
	if ch.remort < 4:
		ch.send("\r\n�������� �������.\r\n")
		return
	if email in array_email:
		ch.send("\r\n�� ��� �������� ���� �������!\r\n")
		return
	array_email.append(email)
	with open("misc/emails_gift.txt", "a") as fp:
		fp.write(email + "\n")
	vnum = random.choice(array_vnum)
	obj = load_obj(vnum)
	ch.obj_to_char(obj)
	ch.send("\r\n� ���� ���� �� ���� ���� ������ � �������� {}.\r\n".format(obj.get_pad(3)))
	
def init():
	global array_email, array_vnum
	if os.path.exists("misc/emails_gift.txt"):
		with open("misc/emails_gift.txt", "r") as fp:
			array_email = [line.strip() for line in fp]
	else:
		with open("misc/emails_gift.txt", "w") as fp:
			pass
	with open("misc/vnums_gift.lst", "r") as fp:
		array_vnum = [int(line.strip()) for line in fp]
		
		
		
		
		