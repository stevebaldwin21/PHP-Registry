#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_vm.h"
#include "zend_exceptions.h"


/*
* Copyright (c) 2016 Steve Baldwin <stevebaldwin21@googlemail.com>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
* and to permit persons to whom the Software is furnished to do so, subject to the following conditions;
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* see https://wiki.php.net/rfc/fast_zpp
* 
* @author Steve Baldwin <stevebaldwin21@googlemail.com>
* Report bugs to the above email address
*/

#define GET_PROPERTY(c, s) { ZVAL_COPY(return_value, zend_read_property(c, getThis(), s, strlen(s), 0, 0), 0, 1); }
#define GET_STATIC_PROPERTY(c, s) { ZVAL_COPY(return_value, zend_read_static_property(c, s, strlen(s), 0, 0), 0, 1); }
#define RETURN_THIS { ZVAL_COPY(return_value, getThis() ); }

#define REGISTRY_ATTR_KEYS 0
#define REGISTRY_ATTR_VALUES 1

#define REGISTRY_NS(cls) "Windows\\Registry\\" ##cls
#define REGISTRY_VAL_NS(cls) REGISTRY_NS("ValueTypes\\") ##cls
#define REGISTRY_EXCEPTION_NS(cls) REGISTRY_NS("Exception\\") ##cls

zend_class_entry ce;

//core classes
zend_class_entry *registry_hkey_local_machine;
zend_class_entry *registry_hive_class;
zend_class_entry *registry_class;
zend_class_entry *registry_value_class;

//exceptions
zend_class_entry *registry_exception_invalid_key;
zend_class_entry *registry_exception_access_denied;
zend_class_entry *registry_exception_invalid_arg;

//value types
zend_class_entry *registry_reg_sz_class;
zend_class_entry *registry_reg_none_class;
zend_class_entry *registry_reg_dword_class;
zend_class_entry *registry_reg_binary_class;

