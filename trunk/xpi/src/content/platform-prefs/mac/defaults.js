/*
 * eID Middleware Project.
 * Copyright (C) 2008-2011 FedICT.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.
 */

// Windows specific default preferences
 
if(!be) var be={};
if(!be.fedict) be.fedict={};
if(!be.fedict.belgiumeidpkcs11) be.fedict.belgiumeidpkcs11={};
if(!be.fedict.belgiumeidpkcs11.defaultPrefs) be.fedict.belgiumeidpkcs11.defaultPrefs={};

be.fedict.belgiumeidpkcs11.defaultPrefs.moduleLocation = "/Library/Frameworks/BeId.framework/Versions/4.0/lib/libbeidpkcs11.dylib;/usr/local/lib/beid-pkcs11.bundle;/usr/local/lib/beid-pkcs11.bundle/Contents/MacOS/libbeidpkcs11.dylib;";