Virtustan MUD (VMUD) readme from Prool

This is Virtustan MUD - multiusers dungeon (MUD) engine (MUD setver).
VMUD is derivative of Byliny (Bylins) MUD.
Byliny is derivative of Circle MUD.

Language of VMUD and Byliny is Russian.

---

Как самому собрать VMUD (или Былины)

Смотри оригинальный readme.markdown. Без проблем собирается в Ubuntu 64 bit или в Windows 64 bit
в средах cygwin или WSL.

Кратко:

Распаковать

Войти в каталог мада (тот, в котором данный файл и файл CMakeLists.txt)

mkdir build

cp cmake1.sh build

cd build

./cmake1.sh

make

На данном этапе может оказаться, что не хватает каких-то библиотек. Тогда читайте оригинальный readme в каталоге old

mkdir ../bin

cp circle ../bin

---

Также я смог собрать этот код в Windows 7 32 bit и macOS Catalina.

---

Как запустить

Создать каталог bin, если его нету и скопировать туда исполняемый файл circle, появившийся в каталоге build
после команды make.

Затем надо набрать bin/circle (находясь в том каталоге, в котором находится данный файл и подкалог bin).
И мад запустится на порту 4000.

А если набрать bin/circle 3000
то запустится на порту 3000

Примечание: для сборки в среде 32-разрядного cygwin (то есть в 32-разрядных Виндах) надо перед тем, как делать
make, раскомментировать строку

//#define CYGWIN32

расположенную в начале файла db.cpp

(если она там есть конечно)

Двоичный файл, полученный внутри cygwin, будет запускаться только из среды cygwin. Чтобы запускать его из Виндовс, придется
найти в положить в каталог bin нужные для работы dll файлы (в количестве около пяти). Какие файлы нужны, скажет сам бинарник,
при запуске, когда выдаст сообщени об ошибке типа "Не могу запуститься, ищу файл cygwin.dll). Все эти файлы берутся из
каталога c:/cygwin/bin

Адрес репозитория Virtustan MUD https://github.com/Zerkalo-MUD/mud/tree/prool-virtustan

---

Пруль, 2024 год, февраль, Харьков, война
http://zerkalo.kharkov.org
http://virtustan.net

Слава Україні!
