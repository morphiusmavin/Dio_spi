if [[ -z "${CROSS_COMPILE}" ]]; then
  echo "run setARMpath.sh"
  exit 1
fi

unzip -o ~/Dio_spi.zip
make -f make_dio1  &> out.txt

if grep -q error out.txt
 then
  find2 error out.txt
  exit 1
fi

if grep -q undefined out.txt
 then
  find2 undefined out.txt
  exit 1
fi
cat out.txt
echo "ftp for dio_slave"
ftp 192.168.88.145
make -f make_dio1 clean

make -f make_dio2  &> out.txt

if grep -q error out.txt
 then
  find2 error out.txt
  exit 1
fi

if grep -q undefined out.txt
 then
  find2 undefined out.txt
  exit 1
fi
cat out.txt
echo "ftp for dio_master"
ftp 192.168.88.151
make -f make_dio2 clean
