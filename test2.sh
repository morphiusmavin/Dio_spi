if [[ -z "${CROSS_COMPILE}" ]]; then
  echo "run setARMpath.sh"
  exit 1
fi

unzip -o ~/temp.zip
cp ~/dev/Truck_Project1/mytypes.h .
make -f make_dio  &> out.txt

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
ftp 192.168.88.145
make -f make_dio clean
