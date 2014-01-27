cd kdwsdl2cpp
kdwsdl2cpp.exe -o ..\transport\wsdl_service.h ..\service.xml
kdwsdl2cpp.exe -o ..\transport\wsdl_service.cpp -impl ..\transport\wsdl_service.h ..\service.xml
PAUSE