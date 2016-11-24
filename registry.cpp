#include "registry.h"
#include "exceptions.h"
#include "zend_interfaces.h"

int get_subkey_attr(HKEY hive, int attribute)
{
	TCHAR    achKey[255];
	DWORD    cbName;
	TCHAR    achClass[MAX_PATH] = TEXT("");
	DWORD    cchClassName = MAX_PATH;
	DWORD    numberOfSubKeys = 0;
	DWORD    cbMaxSubKey;
	DWORD    cchMaxClass;
	DWORD    numberOfValues;
	DWORD    cchMaxValue;
	DWORD    cbMaxValueData;
	DWORD    cbSecurityDescriptor;
	FILETIME ftLastWriteTime;
	int response = 0;

	response = RegQueryInfoKey(
		hive,
		achKey,
		&cchClassName,
		NULL,
		&numberOfSubKeys,
		&cbMaxSubKey,
		&cchMaxClass,
		&numberOfValues,
		&cchMaxValue,
		&cbMaxValueData,
		&cbSecurityDescriptor,
		NULL
	);
	
	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		return 0;
	}

	if (attribute == REGISTRY_ATTR_VALUES)
	{
		return numberOfValues;
	}
	else if (attribute == REGISTRY_ATTR_KEYS)
	{
		return numberOfSubKeys;
	}
	else
	{
		return 0;
	}
}
HKEY zval_to_hkey(zval * val) {
	return ((HKEY)(ULONG_PTR)((LONG)val->value.lval));
}
bool zeq(zval * a, char * b) {
	if (Z_TYPE_P(a) == IS_STRING) {
		if (stricmp(Z_STRVAL_P(a), b) == 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return false;
	}
}
char * zval_hkey_to_string(zval * val)
{
	HKEY expectedHkey = ((HKEY)(ULONG_PTR)((LONG)val->value.lval));

	if (expectedHkey == HKEY_CLASSES_ROOT)
	{
		return "HKEY_CLASSES_ROOT";
	}
	else if (expectedHkey == HKEY_USERS)
	{
		return "HKEY_USERS";
	}
	else if (expectedHkey == HKEY_LOCAL_MACHINE)
	{
		return "HKEY_LOCAL_MACHINE";
	}
	else if (expectedHkey == HKEY_CURRENT_CONFIG)
	{
		return "HKEY_CURRENT_CONFIG";
	}
	else if (expectedHkey == HKEY_CURRENT_USER)
	{
		return "HKEY_CURRENT_USER";
	}
	else
	{
		return "UNKNOWN_HKEY";
	}
}

/*
*
* Registry Hive functions
*
*/

/* {{{ proto RegistryKey Registry::LocalMachine() */
PHP_METHOD(RegistryHive, LocalMachine)
{
	zval registry_key_class;
	object_init_ex(&registry_key_class, registry_class);
	zend_update_property_long(registry_class, &registry_key_class, "hive", strlen("hive"), HKEY_LOCAL_MACHINE);
	zend_update_property_string(registry_class, &registry_key_class, "name", strlen("name"), "HKEY_LOCAL_MACHINE");
	RETURN_ZVAL(&registry_key_class, 0, 1);
}
/* }}} */

/* {{{ proto RegistryKey Registry::CurrentConfig() */
PHP_METHOD(RegistryHive, CurrentConfig)
{
	zval registry_key_class;
	object_init_ex(&registry_key_class, registry_class);
	zend_update_property_long(registry_class, &registry_key_class, "hive", strlen("hive"), (long)HKEY_CURRENT_CONFIG);
	zend_update_property_string(registry_class, &registry_key_class, "name", strlen("name"), "HKEY_CURRENT_CONFIG");
	RETURN_ZVAL(&registry_key_class, 0, 1);
}
/* }}} */

/* {{{ proto RegistryKey Registry::CurrentUser() */
PHP_METHOD(RegistryHive, CurrentUser)
{
	zval registry_key_class;
	object_init_ex(&registry_key_class, registry_class);
	zend_update_property_long(registry_class, &registry_key_class, "hive", strlen("hive"), (long)HKEY_CURRENT_USER);
	zend_update_property_string(registry_class, &registry_key_class, "name", strlen("name"), "HKEY_CURRENT_USER");
	RETURN_ZVAL(&registry_key_class, 0, 1);
}
/* }}} */

/* {{{ proto RegistryKey Registry::ClassesRoot() */
PHP_METHOD(RegistryHive, ClassesRoot)
{
	zval registry_key_class;
	object_init_ex(&registry_key_class, registry_class);
	zend_update_property_long(registry_class, &registry_key_class, "hive", strlen("hive"), (long)HKEY_CLASSES_ROOT);
	zend_update_property_string(registry_class, &registry_key_class, "name", strlen("name"), "HKEY_CLASSES_ROOT");
	RETURN_ZVAL(&registry_key_class, 0, 1);
}
/* }}} */

/*
*
* Registry Key functions
*
*/

/* {{{ proto RegistryKey Registry::__construct(long node) */
PHP_METHOD(Registry, __construct)
{
	windows_throw_exception(windows_invalid_operation_exception, "This class cannot be instantiated manually, please use RegistryHive.");
	return;
}
/* }}} */

/* {{{ proto array Registry::getSubKeyNames(void) */
PHP_METHOD(Registry, getSubKeyNames)
{
	zval subKeys;
	array_init(&subKeys);
	zval * _this = getThis();
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	HKEY rootNode = zval_to_hkey(_property);

	_property = NULL;
	efree(_property);

	int returnCode = 0;

	if (rootNode == NULL) {
		//this should not be reached! otherwise logic is wrong elsewehre.
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
		RETURN_FALSE;
	}

	int cSubKeys = get_subkey_attr(rootNode, REGISTRY_ATTR_KEYS);

	if (cSubKeys > 0)
	{
		TCHAR registryKeyName[255];
		DWORD cbName;

		for (size_t i = 0; i < cSubKeys; i++)
		{
			//max size is 255 of a registry key!
			cbName = 255;
			returnCode = RegEnumKeyEx(rootNode, i, registryKeyName, &cbName, NULL, NULL, NULL, NULL);

			if (returnCode == ERROR_SUCCESS || returnCode == ERROR_MORE_DATA) {
				if (*registryKeyName == '\0') {
					//this is a sub node - but just the defualt one
					add_next_index_string(&subKeys, "Default");
				}
				else {
					add_next_index_string(&subKeys, registryKeyName);
				}
			}
			else
			{
				windows_throw_exception_hres(returnCode);
			}
		}

		cbName = NULL;
		efree(cbName);

		//return [,,,,]
		RETURN_ARR(subKeys.value.arr);
	}
	else
	{
		//return []
		RETURN_ARR(subKeys.value.arr);
	}
}
/* }}} */

/* {{{ proto array Registry::getValueNames(void) */
PHP_METHOD(Registry, getValueNames)
{
	zval subKeys;
	array_init(&subKeys);

	zval * _this = getThis();
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	HKEY rootNode = zval_to_hkey(_property);

	if (rootNode == NULL) {
		//current state is screwed!
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
	}

	int CValues = get_subkey_attr(rootNode, REGISTRY_ATTR_VALUES);

	if (CValues > 0)
	{
		TCHAR achKey[255];
		DWORD cbName;
		LPDWORD data = (LPDWORD)malloc(sizeof(LPDWORD));

		for (size_t i = 0; i < CValues; i++)
		{
			int retCode = RegEnumValue(rootNode, i, achKey, &cbName, NULL, NULL, NULL, data);

			if (retCode == ERROR_SUCCESS || retCode == ERROR_MORE_DATA)
			{
				if (*achKey == '\0')
				{
					add_next_index_string(&subKeys, "Default");
				}
				else
				{
					add_next_index_string(&subKeys, achKey);
				}
			}
		}

		RETURN_ARR(subKeys.value.arr);
	}
	else
	{
		//return []
		RETURN_ARR(subKeys.value.arr);
	}
}
/* }}} */

/* {{{ proto RegistryKey Registry::openSubKey(string key) */
PHP_METHOD(Registry, openSubKey)
{
	zval * _this = getThis();

	char * subKey = (char*)emalloc(sizeof(char*));
	int subKeyLen;

	object_init_ex(return_value, registry_class);
	HKEY temporaryHkeyPointer;
	LPCSTR lpSubKey;
	HKEY rootNode;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING_EX(subKey, subKeyLen, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	//acts as the constructor for this object
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	zend_update_property_long(registry_class, return_value, "hive", strlen("hive"), _property->value.lval);
	zend_update_property_string(registry_class, return_value, "name", strlen("name"), subKey);

	lpSubKey = subKey;
	rootNode = zval_to_hkey(_property);

	if (rootNode == NULL) {
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
	}

	//only return class if RegOpenKey is safe!
	int response = RegOpenKeyEx(rootNode, lpSubKey, 0, KEY_ALL_ACCESS, &temporaryHkeyPointer);

	if (response > 0) {
		//deterimne what caused it!
		windows_throw_exception_hres(response);
	}

	zend_update_property_long(registry_class, return_value, "hive", strlen("hive"), (long)temporaryHkeyPointer);

	//return the zend ce
	RETURN_ZVAL(return_value, 0, 0);
}
/* }}} */

/* {{{ proto RegistryKey::deleteValue(string $valueKey) */
PHP_METHOD(Registry, deleteValue) {

	zval * _this = getThis();

	char * subKey = (char*)emalloc(sizeof(char*));
	int subKeyLen;

	HKEY temporaryHkeyPointer;
	LPCSTR lpSubKey;
	HKEY rootNode;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING_EX(subKey, subKeyLen, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	//acts as the constructor for this object
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);

	int response = RegOpenKeyEx(zval_to_hkey(_property), NULL, 0, KEY_WRITE, &temporaryHkeyPointer);

	_this = NULL;
	efree(_this);

	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	response = RegDeleteValue(temporaryHkeyPointer, subKey);

	subKey = NULL; //esp. malloc!
	free(subKey);

	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto RegistryKey::deleteKey(string $valueKey) */
PHP_METHOD(Registry, deleteKey) {
	zval * _this = getThis();

	char * subKey = (char*)emalloc(sizeof(char*));
	int subKeyLen;

	HKEY temporaryHkeyPointer;
	LPCSTR lpSubKey;
	HKEY rootNode;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING_EX(subKey, subKeyLen, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	//acts as the constructor for this object
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);

	int response = RegOpenKeyEx(zval_to_hkey(_property), NULL, 0, KEY_WRITE, &temporaryHkeyPointer);

	_this = NULL;
	efree(_this);



	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	response = RegDeleteKey(temporaryHkeyPointer, subKey);

	subKey = NULL; //esp. malloc!
	free(subKey);

	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto RegistryKey::close(void) */
PHP_METHOD(Registry, close)
{
	zval * _this = getThis();
	zval * hive = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	HKEY key = zval_to_hkey(hive);

	int response = RegCloseKey(key);

	//just for good measure!
	_this = NULL;
	hive = NULL;
	efree(hive);
	efree(_this);

	if (response > 0) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */



/* {{{ proto RegistryKey::save(string $filepath) */
PHP_METHOD(Registry, save) 
{
	zval * _this = getThis();
	zval * hive;
	char * filepath;
	HKEY hiveKey;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &filepath) == FAILURE) {
		return;
	}
	
	if (filepath[0] == '\0') {
		windows_throw_exception(windows_invalid_file_exception, "Invalid filepath");
	}

	hive = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	hiveKey = zval_to_hkey(hive);

	if (hiveKey == NULL) {
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
	}
	
	int response = RegSaveKeyEx(hiveKey, filepath, NULL, NULL);
	
	if (response != ERROR_SUCCESS)	{
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */


/* {{{ proto RegistryKey Registry::createSubKey(string key) */
PHP_METHOD(Registry, createSubKey)
{
	zval * _this = getThis();
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	HKEY node = zval_to_hkey(_property);
	char * subKey;
	HKEY returnedHKey;
	HKEY rKey;
	DWORD dwDisposition;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &subKey) == FAILURE) {
		return;
	}

	if (node == NULL) {
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
		RETURN_FALSE;
	}

	int response = RegCreateKeyEx(node, subKey, 0, NULL, 0, KEY_WRITE, NULL, &returnedHKey, &dwDisposition);

	if (response > 0) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	object_init_ex(return_value, registry_class);
	zend_update_property_long(registry_class, return_value, "hive", strlen("hive"), (long)returnedHKey);
	zend_update_property_string(registry_class, return_value, "name", strlen("name"), subKey);
	RETURN_ZVAL(return_value, 0, 1);
}
/* }}} */

/* {{{ proto RegistryKey Registry::setValue(string key, Windows\ValueType\Value value) */
PHP_METHOD(Registry, setValue)
{
	zval * _this = getThis();
	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	HKEY node = zval_to_hkey(_property); //the node directly from the object
	char * subKeyName = (char*)emalloc(sizeof(char*));
	int subKeyNameL;
	zval * subKeyValue = (zval*)emalloc(sizeof(zval*));
	zval * storeZval;
	BYTE *lpData = (BYTE*)malloc(sizeof(BYTE*));
	DWORD cbData;
	DWORD intendedType;
	zend_class_entry * entryObject;
	int response = 0;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STRING_EX(subKeyName, subKeyNameL, 1, 0)
		Z_PARAM_OBJECT_OF_CLASS(subKeyValue, registry_value_class)
	ZEND_PARSE_PARAMETERS_END();

	if (node == NULL) {
		windows_throw_exception(windows_invalid_operation_exception, "Invalid registry key");
	}

	storeZval = zend_read_property(registry_value_class, subKeyValue, "value", strlen("value"), 0, 0);

	switch (Z_TYPE_P(storeZval))
	{
		case IS_STRING:
		{
			convert_to_string(storeZval);
			cbData = (Z_STRLEN_P(storeZval) + 1);
			intendedType = REG_SZ;
			//had to modify this to here because it was setting unicode characters and I couldn't ifugre out why - ohwell...
			response = RegSetValueEx(node, subKeyName, NULL, intendedType, (const BYTE*)TEXT(Z_STRVAL_P(storeZval)), cbData);
		}
		break;
		//if null then set to DWORD 0 
		case IS_NULL:
		{
			lpData = 0;
			cbData = (sizeof(DWORD) + 1);
			intendedType = REG_NONE;
			//you must provide a pointer to the lpData on binary/dword types
			response = RegSetValueEx(node, subKeyName, NULL, intendedType, (const BYTE*)&lpData, cbData);
		}
		break;
		case IS_LONG:
		case IS_DOUBLE:
		{
			//convert_to_long(fZval);
			lpData = (BYTE*)Z_LVAL_P(storeZval);
			DWORD val = Z_LVAL_P(storeZval);
			cbData = sizeof(DWORD);
			intendedType = REG_DWORD;
			//you must provide a pointer to the lpData on binary/dword types
			response = RegSetValueEx(node, subKeyName, NULL, intendedType, (CONST BYTE*)&val, cbData);
		}
		break;
		case IS_ARRAY:
		{
			RETURN_FALSE;
		} break;
	}

	subKeyValue = NULL;
	efree(subKeyValue);

	//something went wrong - find out!
	if (response > 0) {
		windows_throw_exception_hres(response);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto RegistryKey Registry::getValue([,string key]) */
PHP_METHOD(Registry, getValue)
{
	zval * _this = getThis();
	char * subKey = (char*)emalloc(sizeof(char*));
	zval registry_node;
	DWORD dwType;
	char  registryValue[255];
	DWORD dwDataSize = 255;
	HKEY temporaryHkeyPointer;
	zval registry_value;
	zval new_class;
	int response = 0;

	//set to nothing
	strcpy(subKey, "");

	//the registry value class
	object_init_ex(&registry_value, registry_value_class);

	zval * _property = zend_read_property(registry_class, _this, "hive", strlen("hive"), 0, 0);
	zval * name = zend_read_property(registry_class, _this, "name", strlen("hive"), 0, 0);

	//optional string argument otherwise NULL is returned
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|s", &subKey) == FAILURE) { return; }

	response = RegOpenKeyEx(zval_to_hkey(_property), NULL, 0, KEY_READ, &temporaryHkeyPointer);

	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	//set back!
	response = 0;

	if (*subKey != '\0')
	{
		response = RegQueryValueEx(temporaryHkeyPointer, subKey, NULL, &dwType, (LPBYTE)registryValue, &dwDataSize);
	}
	else
	{
		response = RegQueryValueEx(temporaryHkeyPointer, NULL, NULL, &dwType, (LPBYTE)registryValue, &dwDataSize);
	}

	subKey = NULL;
	efree(subKey);

	if (response != ERROR_SUCCESS) {
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	//clearup!

	switch (dwType)
	{
	case REG_SZ || REG_MULTI_SZ || REG_EXPAND_SZ:
	{
		object_init_ex(&new_class, registry_reg_sz_class);

		if (dwDataSize == 1)
		{
			//its empty! \0
			zend_update_property_null(registry_value_class, &new_class, "value", strlen("value"));
		}
		//otherwise its too big!
		else if (dwDataSize < 255)
		{
			zend_update_property_string(registry_value_class, &new_class, "value", strlen("value"), registryValue);
		}

		RETURN_ZVAL(&new_class, 0, 1);
	}
	break;
	case REG_NONE:
	{
		object_init_ex(&new_class, registry_reg_none_class);
		zend_update_property_null(registry_value_class, &new_class, "value", strlen("value"));
		RETURN_ZVAL(&new_class, 0, 1);
	}
	break;
	case REG_BINARY:
	{
		object_init_ex(&new_class, registry_reg_binary_class);
		zend_update_property_string(registry_value_class, &new_class, "value", strlen("value"), registryValue);
		RETURN_ZVAL(&new_class, 0, 1);
	}
	break;
	case REG_QWORD:
	{
		DWORD * n = (DWORD*)malloc(sizeof(DWORD*));
		zval dwordZval;
		ZVAL_LONG(&dwordZval, *(DWORD *)registryValue);
		object_init_ex(&new_class, registry_reg_dword_class);
		zend_update_property(registry_value_class, &new_class, "value", strlen("value"), &dwordZval);
		zval_dtor(&dwordZval);
		efree(n);
		RETURN_ZVAL(&new_class, 0, 1);
	}
	break;
	case REG_DWORD:
	{
		DWORD * n = (DWORD*)malloc(sizeof(DWORD*));
		zval dwordZval;
		ZVAL_LONG(&dwordZval, *(DWORD *)registryValue);
		object_init_ex(&new_class, registry_reg_dword_class);
		zend_update_property(registry_value_class, &new_class, "value", strlen("value"), &dwordZval);
		zval_dtor(&dwordZval);
		efree(n);
		RETURN_ZVAL(&new_class, 0, 1);
	}
	break;
	}

	//should NOT reach here!
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool Registry::hasSubKeys(void) */
PHP_METHOD(Registry, hasSubKeys)
{
	zval keysArray;

	//maybe not the best way to handle this!
	zim_Registry_getSubKeyNames(execute_data, &keysArray);
	int size = zend_array_count(keysArray.value.arr);
	zval_dtor(&keysArray);

	if (size > 0)
	{
		RETURN_TRUE;
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool Registry::subKeyExists(void) */
PHP_METHOD(Registry, subKeyExists) 
{
	zval * _this = getThis();
	zval valuesArray;
	int size;
	zval * iterator;
	char * subKey = NULL;
	int subKeyLength;
	bool result = false;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STRING_EX(subKey, subKeyLength, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	// this will not work? zend_call_method_with_0_params(_this, Z_OBJCE_P(_this), NULL, "getSubKeyNames", &valuesArray);

	zim_Registry_getSubKeyNames(execute_data, &valuesArray);
	size = zend_array_count(valuesArray.value.arr);

	if (size <= 0)	{
		RETURN_FALSE;
	}

	if (subKey[0] == '\0') {
		windows_throw_exception(windows_invalid_argument_exception, "Invalid subkey provided");
		RETURN_FALSE;
	}

	ZEND_HASH_FOREACH_VAL(valuesArray.value.arr, iterator)
		if (zeq(iterator, subKey)) {
			result = true;
			goto before_exit;
		}
	ZEND_HASH_FOREACH_END();

before_exit:
	subKey = NULL;
	efree(subKey);
	zval_dtor(&valuesArray);
	iterator = NULL;
	efree(iterator); //clean up before leaving
	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto bool Registry::hasValues(void) */
PHP_METHOD(Registry, hasValues)
{
	zval valuesArray;
	zim_Registry_getValueNames(execute_data, &valuesArray);
	int size = zend_array_count(valuesArray.value.arr);
	zval_dtor(&valuesArray);

	if (size > 0)
	{
		RETURN_TRUE;
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string Registry::getName(void) */
PHP_METHOD(Registry, getName)
{
	GET_PROPERTY(registry_class, "name");
}
/* }}} */

/* {{ proto ValueTypes\Value Value::getType(void) */
PHP_METHOD(RegistryValue, getType)
{
	zval * _this = getThis();
	zend_class_entry *ce = Z_OBJCE_P(_this);

	_this = NULL;
	efree(_this);

	ZVAL_STR(return_value, ce->name);
}
/* }}} */

/* {{{ proto ValueTypes\Value Value::getValue(string key) */
PHP_METHOD(RegistryValue, getValue)
{
	GET_PROPERTY(registry_class, "value");
}
/* }}} */

/*
*
* Value type methods
*
*/

/* {{{ proto Sz::__construct(string value) */
PHP_METHOD(Sz, __construct) {
	char * value;
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &value) == FAILURE) { return; }
	zend_update_property_string(registry_reg_sz_class, getThis(), "value", strlen("value"), value);
}
/* }}} */

/* {{{ proto Dword::__construct(string value) */
PHP_METHOD(Dword, __construct)
{
	long * value;
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &value) == FAILURE) { return; }

	if (value < 1 || value > MAXDWORD32)
	{
		windows_throw_exception(windows_invalid_argument_exception, "Value is too large or too small");
		return;
	}

	zend_update_property_long(registry_reg_dword_class, getThis(), "value", strlen("value"), value);
}
/* }}} */

/* {{{ proto Binary::__construct(string value) */
PHP_METHOD(Binary, __construct)
{
	long * value;
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &value) == FAILURE) { return; }
	zend_update_property_long(registry_reg_sz_class, getThis(), "value", strlen("value"), value);
}
/* }}} */


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_rh_lm, 0, 0, IS_OBJECT, REGISTRY_NS("RegistryKey"), 0)
ZEND_END_ARG_INFO()

zend_function_entry registry_hive_functions[] = {
	PHP_ME(RegistryHive, LocalMachine, ai_rh_lm, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(RegistryHive, CurrentConfig, ai_rh_lm, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(RegistryHive, ClassesRoot, ai_rh_lm, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(RegistryHive, CurrentUser, ai_rh_lm, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_value_get_type, 0, 0, IS_STRING, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_value_get_value, 0, 0, IS_OBJECT, REGISTRY_VAL_NS("Value"), 0)
ZEND_END_ARG_INFO()

//class defintions
zend_function_entry registry_value_functions[] = {
	PHP_ME(RegistryValue, getValue, ai_registry_value_get_value, ZEND_ACC_PUBLIC)
	PHP_ME(RegistryValue, getType, ai_registry_value_get_type, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_setvalue, 0, 2, _IS_BOOL, NULL, 0)
ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, value, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_get_value, 0, 1, IS_OBJECT, REGISTRY_VAL_NS("Value"), 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_get_subkey_names, 0, 1, IS_ARRAY, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_get_subkey_values, 0, 1, IS_ARRAY, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_has_subkeys, 0, 1, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_has_values, 0, 1, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_create_subkey, 0, 1, IS_OBJECT, REGISTRY_NS("RegistryKey"), 0)
ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_open_subkey, 0, 1, IS_OBJECT, REGISTRY_NS("RegistryKey"), 0)
ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_delete_value, 0, 0, _IS_BOOL, NULL, 0)
ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_registry_delete_key, 0, 0, _IS_BOOL, NULL, 0)
ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

zend_function_entry registry_key_functions[] = {
	PHP_ME(Registry, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, getSubKeyNames, ai_registry_get_subkey_names, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, getValueNames, ai_registry_get_subkey_values, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, getValue, ai_registry_get_value, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, setValue, ai_registry_setvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, openSubKey, ai_registry_open_subkey, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, hasSubKeys, ai_registry_has_subkeys, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, hasValues, ai_registry_has_values, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, createSubKey, ai_registry_create_subkey, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, deleteValue, ai_registry_delete_value, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, deleteKey, ai_registry_delete_key, ZEND_ACC_PUBLIC)
	PHP_ME(Registry, subKeyExists, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

ZEND_BEGIN_ARG_INFO_EX(ai_registry_value_constructor, 0, 0, 1)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

zend_function_entry registry_sz_functions[] = {
	PHP_ME(Sz, __construct, ai_registry_value_constructor, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_function_entry registry_dword_functions[] = {
	PHP_ME(Dword, __construct, ai_registry_value_constructor, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_function_entry registry_binary_functions[] = {
	PHP_ME(Binary, __construct, ai_registry_value_constructor, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

PHP_MINIT_FUNCTION(WindowsRegistry)
{
	INIT_CLASS_ENTRY(ce, REGISTRY_NS("RegistryHive"), registry_hive_functions);
	registry_hive_class = zend_register_internal_class(&ce TSRMLS_CC);
	registry_hive_class->ce_flags |= ZEND_ACC_FINAL;
	registry_hive_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	INIT_CLASS_ENTRY(ce, REGISTRY_NS("RegistryKey"), registry_key_functions);
	registry_class = zend_register_internal_class(&ce TSRMLS_CC);
	registry_class->ce_flags |= ZEND_ACC_FINAL;

	//this class cannot be final!
	INIT_CLASS_ENTRY(ce, REGISTRY_VAL_NS("Value"), registry_value_functions);
	registry_value_class = zend_register_internal_class(&ce);
	registry_value_class->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, REGISTRY_VAL_NS("Sz"), registry_sz_functions);
	registry_reg_sz_class = zend_register_internal_class_ex(&ce, registry_value_class);
	registry_reg_sz_class->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce, REGISTRY_VAL_NS("None"), NULL);
	registry_reg_none_class = zend_register_internal_class_ex(&ce, registry_value_class);
	registry_reg_none_class->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce, REGISTRY_VAL_NS("Dword"), registry_dword_functions);
	registry_reg_dword_class = zend_register_internal_class_ex(&ce, registry_value_class);
	registry_reg_dword_class->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce, REGISTRY_VAL_NS("Binary"), registry_binary_functions);
	registry_reg_binary_class = zend_register_internal_class_ex(&ce, registry_value_class);
	registry_reg_binary_class->ce_flags |= ZEND_ACC_FINAL;

	zend_declare_property_null(registry_reg_none_class, "value", strlen("value"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(registry_class, "hive", strlen("hive"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(registry_class, "name", strlen("name"), 0, ZEND_ACC_PUBLIC);
	zend_declare_property_long(registry_value_class, "hive", strlen("hive"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_string(registry_value_class, "value", strlen("value"), "", ZEND_ACC_PUBLIC);
	zend_declare_property_long(registry_hive_class, "hive", strlen("hive"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_string(registry_hive_class, "value", strlen("value"), "", ZEND_ACC_PUBLIC);

	SUCCESS;
}

zend_module_entry WindowsRegistry_module_entry = {
	STANDARD_MODULE_HEADER,
	"WindowsRegistry",
	NULL,
	PHP_MINIT(WindowsRegistry), NULL, NULL, NULL, NULL,
	NO_VERSION_YET, STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(WindowsRegistry)


