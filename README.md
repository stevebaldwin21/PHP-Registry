# PHP Windows Registry

This repository is for a work in progress OOP windows registry PHP extension for PHP7. Currently there is only global functions to access registry like the PECL extension which has not been actively developed since PHP 5.4.

```php

use Windows\Registry\RegistryHive;
use Windows\Registry\RegistryKey;
use Windows\Registry\RegistryValue;
use Windows\Registry\Exception\InvalidKeyException;
use Windows\Registry\ValueTypes\Sz;
use Windows\Registry\ValueTypes\None;
use Windows\Registry\ValueTypes\Binary;
use Windows\Registry\ValueTypes\Dword;

$registry = RegistryHive::CurrentMachine(); //instanceof Windows\Registry\RegistryKey representing the HKEY_CURRENT_MACHINE

try 
{
  $key = $registry->openSubKey("System\\HardwareConfig");
  //or you can use the return value to traverse
  $key = $registry->openSubKey("System")->openSubKey("HardwareConfig");
  
  if ($key->hasSubKeys() == false) 
  {
    //handle this..
  }
  
  $value = $key->getValue("LastConfig"); //instanceof ValueTypes\Type 

  if ($value instanceof Sz) 
  {
    printf("Your value: %s", $value->getValue());
  }
  else if ($value instanceof Binary) 
  {
    printf("Your binary value: %s", $value->getValue());
  }
  else if ($value instanceof Dword) 
  {
      printf("Your dword value: %d", $value->getValue());
  }
  else
  {
    //expected Sz instance!
  }
}
catch(InvalidKeyException $e) 
{
  //handle me..
}
```

Creating sub keys

```php
use Windows\Registry\RegistryHive;
use Windows\Registry\RegistryKey;
use Windows\Registry\RegistryValue;
use Windows\Registry\Exception\InvalidKeyException;

$registry = RegistryHive::LocalMachine();

$registry->createSubKey("Hello")->createSubKey("Test");
//or
$registry->createSubKey("Hello\\Test");
```

Setting values

```php
use Windows\Registry\RegistryHive;
use Windows\Registry\RegistryKey;
use Windows\Registry\RegistryValue;
use Windows\Registry\Exception\InvalidKeyException;

$registry = RegistryHive::LocalMachine();

$software = $registry->openSubKey("Software");
$software->setValue("TestKey", 123); //DWORD 0x000007b (123)
$software->setValue("TestKey", "hello"); //SZ "hello"

```


Enumeration

```php
use Windows\Registry\RegistryHive;
use Windows\Registry\RegistryKey;
use Windows\Registry\RegistryValue;
use Windows\Registry\Exception\InvalidKeyException;
use Windows\Registry\ValueTypes\Sz;
use Windows\Registry\ValueTypes\None;
use Windows\Registry\ValueTypes\Binary;
use Windows\Registry\ValueTypes\Dword;

$registry = RegistryHive::LocalMachine();

$names = $registry->getSubKeyNames(); //array of all the subkeys
$values = $registry->getValueNames(); //array of the value names

$hasSubKeys = $registry->hasSubKeys();
$hasValues = $registry->hasValues(); //returns 0 if only default is present and is '(not set)'
```


