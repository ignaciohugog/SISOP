#/bin/bash

make

export 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-20131c-not-an-issue-but-a-feature/Comunes/Release'
export 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-20131c-not-an-issue-but-a-feature/memoria/Release'
export 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-20131c-not-an-issue-but-a-feature/so-commons-library/Release'

cp /home/utnso/tp-20131c-not-an-issue-but-a-feature/Plataforma/koopa /home/utnso/tp-20131c-not-an-issue-but-a-feature/Plataforma/Release
cp /home/utnso/tp-20131c-not-an-issue-but-a-feature/memoria/tests/7_test_beyond_end.lst /home/utnso/tp-20131c-not-an-issue-but-a-feature/Plataforma/Release
cp /home/utnso/tp-20131c-not-an-issue-but-a-feature/Plataforma/PLANIFICADOR.cfg /home/utnso/tp-20131c-not-an-issue-but-a-feature/Plataforma/Release

rm /lib/libso-commons-library.so
rm /lib/libmemoria.so

ln -s /home/utnso/workspace/tp-20131c-not-an-issue-but-a-feature/so-commons-library/Release/libso-commons-library.so /lib/libso-commons-library.so
ln -s /home/utnso/workspace/tp-20131c-not-an-issue-but-a-feature/memoria/Release/libmemoria.so /lib/libmemoria.so