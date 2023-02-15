# -*- coding: koi8-r -*-

"""��������� ������������ �������.

������� ������������ �� ���� ��������� ������������� � ����� ���������� - ���������� �������.
������ ����� ����������� ������������� �� �������, � ��� �� ���������� �� ���� �����.
������ ���� ��� �� ���������� ������� ��� ������ ����� ������.

���������� ������� ������ ����� ����� ���������:
def handle_xxx(event_name, **kwargs):
    ...

���� ���������� ���������� True, ������� ��������� ������������ ������ �� ������� ������������.

������ ������������ ����� ������� ��. � scripting.hpp.
"""

import mud
from utils import log_err


event_listeners = {}

def subscribe(event_name, listener):
    if event_name not in event_listeners:
        event_listeners[event_name] = [listener]
    else:
        event_listeners[event_name].append(listener)


def unsubscribe(event_name, listener):
    try:
        event_listeners[event_name].remove(listener)
    except KeyError:
        log_err("Trying to unsubscribe from non existent event: " + event_name)
    except ValueError:
        log_err("Trying to unsubscribe not subscribed listener from event: " + event_name)


def publish(event_name, **kwargs):
    listeners = event_listeners.get(event_name, [])
    for listener in listeners[:]:
        try:
            result = listener(event_name, **kwargs)
        except:
            log_err("Unhandled exception raised by " + event_name + " event listener")
            continue
        if result:
            break  # handling completed, skip other listeners
