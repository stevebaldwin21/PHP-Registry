#pragma once

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

#define EXCEPTION_NS(cls) "Windows\\Exceptions\\" ##cls

zend_class_entry ce;
zend_class_entry * windows_access_denied;
zend_class_entry * windows_invalid_operation_exception;
zend_class_entry * windows_invalid_file_exception;
zend_class_entry * windows_out_of_bounds_exception;
zend_class_entry * windows_runtime_exception;
zend_class_entry * windows_invalid_argument_exception;
zend_class_entry * windows_illegal_operation_exception;


void init_windows_exceptions();
void throw_windows_exception(zend_class_entry * class);
void windows_throw_exception(zend_class_entry * class, char * message);
void windows_throw_exception_hres(DWORD hResult);
