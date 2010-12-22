﻿/* ****************************************************************************

 * eID Middleware Project.
 * Copyright (C) 2010-2010 FedICT.
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

**************************************************************************** */

using System;
using System.Collections.Generic;

using System.Text;

using System.Runtime.InteropServices;

using Net.Sf.Pkcs11;
using Net.Sf.Pkcs11.Objects;
using Net.Sf.Pkcs11.Wrapper;

using System.Security.Cryptography.X509Certificates;

namespace EidSamples
{
    class ReadData
    {
        private Module m = null;

        public ReadData()
        {
            if (m == null)
            {
                m = Module.GetInstance("beidpkcs11-test.dll");
            }
        }

        public string GetSlotDescription()
        { 
            String slotID;
            m.Initialize();
            try
            {
                // Look for slots (cardreaders)
                // GetSlotList(false) will return all cardreaders
                Slot []slots = m.GetSlotList(false);
                if (slots.Length == 0)
                    slotID = "";
                slotID = slots[0].SlotInfo.SlotDescription.Trim();
            }
            finally
            {
                m.Finalize_();
            }
            return slotID;
        }

        public string GetTokenInfoLabel()
        {
            String tokenInfoLabel;
            m.Initialize();
            try
            {
                // Look for slots (cardreaders)
                // GetSlotList(true) will return only the cardreaders with a 
                // token (smart card)
                tokenInfoLabel = m.GetSlotList(true)[0].Token.TokenInfo.Label.Trim();
            }
            finally
            {
                m.Finalize_();
            }
            return tokenInfoLabel;
            
        }
        public string GetSurname()
        {
            String name;
            m.Initialize();
            try
            {
                // Get the first slot (cardreader) with a token
                Slot slot = m.GetSlotList(true)[0];
                Session session = slot.Token.OpenSession(true);
                
                // Search for objects
                // First, define a search template 
                ByteArrayAttribute surnameLabel = new ByteArrayAttribute(CKA.LABEL);
                surnameLabel.Value = System.Text.Encoding.UTF8.GetBytes("Surname");
                session.FindObjectsInit(new P11Attribute[] {
                        surnameLabel
                    }
                    );

                Net.Sf.Pkcs11.Objects.Data surname = session.FindObjects(1)[0] as Net.Sf.Pkcs11.Objects.Data;

                name = System.Text.Encoding.UTF8.GetString(surname.Value.Value);
                session.FindObjectsFinal();
            }
            finally
            {
                m.Finalize_();
            }
            return name;

        }
        public string GetDateOfBirth()
        {
            String value;
            m.Initialize();
            try
            {
                Slot slot = m.GetSlotList(false)[0];
                Session session = slot.Token.OpenSession(true);
                ByteArrayAttribute dateOfBirthLabel = new ByteArrayAttribute(CKA.LABEL);
                dateOfBirthLabel.Value = System.Text.Encoding.UTF8.GetBytes("Date_Of_Birth");
                session.FindObjectsInit(new P11Attribute[] {
                    dateOfBirthLabel
                }
                    );

                Data dateOfBirth = session.FindObjects(1)[0] as Data;
                //surname.ReadAttributes(session);
                value = System.Text.Encoding.UTF8.GetString(dateOfBirth.Value.Value);
                session.FindObjectsFinal();
            }
            finally
            {
                m.Finalize_();
            }
            return value;
        }
        public byte[] GetIdFile()
        {
            return GetFile("DATA_FILE");
        }
        public byte[] GetIdSignatureFile()
        {
            return GetFile("SGN_RN");
        }

        private byte[] GetFile(String Filename)
        {
            byte[] value;
            m.Initialize();
            try
            {
                Slot slot = m.GetSlotList(false)[0];
                Session session = slot.Token.OpenSession(true);
                ByteArrayAttribute idFileLabel = new ByteArrayAttribute(CKA.LABEL);
                idFileLabel.Value = System.Text.Encoding.UTF8.GetBytes(Filename);
                session.FindObjectsInit(new P11Attribute[] {
                    idFileLabel
                }
                    );

                Data idFile = session.FindObjects(1)[0] as Data;
                //surname.ReadAttributes(session);
                value = idFile.Value.Value;
                session.FindObjectsFinal();
            }
            finally
            {
                m.Finalize_();
            }
            return value;
        }

        public List <string> GetCertificateLabels()
        {
            m.Initialize();
            List<string> labels = new List<string>();
            try
            {
                Slot slot = m.GetSlotList(false)[0];
                Session session = slot.Token.OpenSession(true);
                ObjectClassAttribute certificateAttribute = new ObjectClassAttribute(CKO.CERTIFICATE);
                session.FindObjectsInit(new P11Attribute[] {
                     certificateAttribute
                    }
                );


                P11Object[] certificates = session.FindObjects(100) as P11Object[];
                foreach (P11Object certificate in certificates)
                {
                    labels.Add (new string (((X509PublicKeyCertificate)certificate).Label.Value));
                }
                
                //certs. new X509Certificate(certificate.Value.Value);
                
                session.FindObjectsFinal();
            }
            finally
            {
                m.Finalize_();
            }
            return labels;
        }
    }
}
