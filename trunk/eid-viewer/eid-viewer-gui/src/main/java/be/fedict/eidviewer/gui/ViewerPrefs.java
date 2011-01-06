/*
 * eID Middleware Project.
 * Copyright (C) 2010 FedICT.
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

package be.fedict.eidviewer.gui;

import java.util.prefs.Preferences;

/**
 *
 * @author Frank Marien
 */
public class ViewerPrefs
{
    public static final String          AUTO_VALIDATE_TRUST              = "auto_validate_trust";
    public static final String          TRUSTSERVICE_PROTO               = "trust_service_proto";
    public static final String          TRUSTSERVICE_URI                 = "trust_service_uri";
    public static final String          HTTP_PROXY_ENABLE                = "enable_http_proxy";
    public static final String          HTTP_PROXY_HOST                  = "http_proxy_host";
    public static final String          HTTP_PROXY_PORT                  = "http_proxy_port";

    public static final boolean         DEFAULT_HTTP_PROXY_ENABLE       = false;
    public static final boolean         DEFAULT_AUTO_VALIDATE_TRUST     = false;

    public static final String          DEFAULT_TRUSTSERVICE_PROTO      = "http";
    public static final String          DEFAULT_TRUSTSERVICE_URI        = "trust.services.belgium.be";
    public static final String          DEFAULT_HTTP_PROXY_HOST         = "";
    public static final int             DEFAULT_HTTP_PROXY_PORT         = 8080;
    
    private static Preferences          preferences;
    private static String               startupHttpProxyHost;
    private static int                  startupHttpProxyPort;
    private static boolean              startupUseHttpProxy;

    public static Preferences getPrefs()
    {
        if(preferences==null)
            preferences=Preferences.userNodeForPackage(ViewerPrefs.class);
        return preferences;
    }

    public static void getStartupProxySettings()
    {
        if(startupHttpProxyHost!=null)
            return;

        startupHttpProxyHost=System.getProperty("http.proxyHost");
        if(startupHttpProxyHost!=null)
        {
            if(System.getProperty("http.proxyPort")!=null)
            {
                try
                {
                    startupHttpProxyPort=Integer.parseInt(System.getProperty("http.proxyPort"));
                    startupUseHttpProxy=true;
                }
                catch(NumberFormatException nfe)
                {
                    // if the system property has a non-numerical value, we just don't set startupUseHttpProxy
                }
            }
        }
    }

    public static boolean getIsAutoValidating()
    {
        return getPrefs()!=null?getPrefs().getBoolean(AUTO_VALIDATE_TRUST, DEFAULT_AUTO_VALIDATE_TRUST):DEFAULT_AUTO_VALIDATE_TRUST;
    }

    public static void setAutoValidating(boolean state)
    {
        if(getPrefs()==null)
            return;
        getPrefs().putBoolean(AUTO_VALIDATE_TRUST, state);
    }

    public static String getTrustServiceProto()
    {
        return getPrefs()!=null?getPrefs().get(TRUSTSERVICE_PROTO, DEFAULT_TRUSTSERVICE_PROTO):DEFAULT_TRUSTSERVICE_PROTO;
    }
  
    public static String getTrustServiceURI()
    {
        return getPrefs()!=null?getPrefs().get(TRUSTSERVICE_URI, DEFAULT_TRUSTSERVICE_URI):DEFAULT_TRUSTSERVICE_URI;
    }

    public static String getTrustServiceURL()
    {
        return getTrustServiceProto() + "://" + getTrustServiceURI();
    }

    public static void setUseHTTPProxy(boolean use)
    {
        if(getPrefs()==null)
            return;
        getPrefs().putBoolean(HTTP_PROXY_ENABLE, use);
    }

    public static boolean getUseHTTPProxy()
    {
        return getPrefs()!=null?getPrefs().getBoolean(HTTP_PROXY_ENABLE, DEFAULT_HTTP_PROXY_ENABLE):DEFAULT_HTTP_PROXY_ENABLE;
    }

    public static void setHTTPProxyHost(String host)
    {
         if(getPrefs()==null)
            return;
        getPrefs().put(HTTP_PROXY_HOST, host);
    }

    public static String getHTTPProxyHost()
    {
        return getPrefs()!=null?getPrefs().get(HTTP_PROXY_HOST, DEFAULT_HTTP_PROXY_HOST):DEFAULT_HTTP_PROXY_HOST;
    }

     public static void setHTTPProxyPort(int port)
    {
         if(getPrefs()==null)
            return;
        getPrefs().putInt(HTTP_PROXY_PORT, port);
    }

    public static int getHTTPProxyPort()
    {
        return getPrefs()!=null?getPrefs().getInt(HTTP_PROXY_PORT, DEFAULT_HTTP_PROXY_PORT):DEFAULT_HTTP_PROXY_PORT;
    }

    public static String getStartupHttpProxyHost()
    {
        getStartupProxySettings();
        return startupHttpProxyHost;
    }

    public static int getStartupHttpProxyPort()
    {
        getStartupProxySettings();
        return startupHttpProxyPort;
    }

    public static boolean getStartupUseHttpProxy()
    {
        getStartupProxySettings();
        return startupUseHttpProxy;
    }
}

