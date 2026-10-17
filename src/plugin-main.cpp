/*
Reject Cookies for OBS
Copyright (C) 2026 Jayconius

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

#include <QApplication>
#include <QMetaObject>
#include <QTimer>
#include <QWidget>

#include "browser-panel.hpp"

OBS_DECLARE_MODULE()

namespace {

// Sent to every browser dock every couple of seconds; the guard makes it a no-op once a page
// has it, so slow-loading docks and reloads are picked up. On twitch.tv it watches the DOM and clicks the banner's reject
// button as soon as it appears. The button is found by position, not label, so it works in
// any Twitch language: it's the one button next to Accept that isn't Accept or Customize.
const char *kInstallScript = R"JS(
(function () {
  if (window.__obsRejectCookies) return;
  if (!/(^|\.)twitch\.tv$/.test(location.hostname)) return;
  window.__obsRejectCookies = true;

  function findReject() {
    var accept = document.querySelector('[data-a-target="consent-banner-accept"]');
    if (!accept) return null;
    for (var n = accept.parentElement; n && n !== document.body; n = n.parentElement) {
      var others = [].filter.call(n.querySelectorAll('button'), function (b) {
        var t = b.getAttribute('data-a-target');
        return t !== 'consent-banner-accept' && t !== 'consent-banner-manage-preferences';
      });
      if (others.length) return others.length === 1 ? others[0] : null;
    }
    return null;
  }

  // A click made the instant the banner renders can be ignored if Twitch isn't ready yet,
  // so keep clicking once a second until the banner is gone.
  var retry = null;
  function tryReject() {
    if (retry) return;
    var btn = findReject();
    if (!btn) return;
    btn.click();
    retry = setInterval(function () {
      var again = findReject();
      if (!again) {
        clearInterval(retry);
        retry = null;
        return;
      }
      again.click();
    }, 1000);
  }

  tryReject();
  new MutationObserver(tryReject).observe(document.documentElement, { childList: true, subtree: true });
})();
)JS";

QTimer *timer = nullptr;

// qobject_cast can't be used across the obs-browser DLL boundary, so match by class name instead.
QCefWidget *asCefWidget(QWidget *w)
{
	for (const QMetaObject *m = w->metaObject(); m; m = m->superClass()) {
		if (qstrcmp(m->className(), "QCefWidget") == 0)
			return static_cast<QCefWidget *>(w);
	}
	return nullptr;
}

void sweep()
{
	for (QWidget *w : QApplication::allWidgets()) {
		if (QCefWidget *cef = asCefWidget(w))
			cef->executeJavaScript(kInstallScript);
	}
}

void onFrontendEvent(enum obs_frontend_event event, void *)
{
	if (event != OBS_FRONTEND_EVENT_FINISHED_LOADING || timer)
		return;
	timer = new QTimer(static_cast<QWidget *>(obs_frontend_get_main_window()));
	QObject::connect(timer, &QTimer::timeout, sweep);
	timer->start(2000);
	sweep();
	obs_log(LOG_INFO, "watching browser docks for Twitch cookie banners");
}

} // namespace

bool obs_module_load(void)
{
	obs_frontend_add_event_callback(onFrontendEvent, nullptr);
	obs_log(LOG_INFO, "plugin loaded (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(onFrontendEvent, nullptr);
	if (timer)
		timer->stop();
	obs_log(LOG_INFO, "plugin unloaded");
}

