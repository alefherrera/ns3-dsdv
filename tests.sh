#Valores default
#sides = 2
#speed = 500
#dataRate = 8;
#package = 1000;
#distance = 100;
#udp = false;
#networkType = 0;
mkdir ../prueba
../waf --run dsdv --cwd=prueba/
mkdir ../prueba2
../waf --run 'dsdv --sides=3' --cwd=prueba2/
mkdir ../prueba3
../waf --run "dsdv --udp=true" --cwd=prueba3/
mkdir ../prueba4
../waf --run "dsdv --sides=3 --networkType=1" --cwd=prueba4/
mkdir ../prueba5
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --speed=0" --cwd=prueba5/
mkdir ../prueba6
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --speed=1000" --cwd=prueba6/
mkdir ../prueba7
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --distance=50" --cwd=prueba7/
mkdir ../prueba8
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --speed=200 --distance=200" --cwd=prueba8/
mkdir ../prueba9
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --distance=200" --cwd=prueba9/
mkdir ../prueba10
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --distance=200 --package=2000" --cwd=prueba10/
mkdir ../prueba11
../waf --run "dsdv --sides=3 --udp=true --networkType=1 --distance=200 --package=2000 --dataRate=6" --cwd=prueba11/