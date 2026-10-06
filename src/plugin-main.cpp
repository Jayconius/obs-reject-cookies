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
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

namespace {

// Runs inside each browser dock. Does nothing unless the page is on twitch.tv and the
// consent banner is showing, so it is safe to run repeatedly.
const char *kRejectScript = R"JS(
(function () {
  if (!/(^|\.)twitch\.tv$/.test(location.hostname)) return;
  var accept = document.querySelector('[data-a-target="consent-banner-accept"]');
  if (!accept) return;
  var isReject = function (b) { return b.innerText.trim() === 'Reject'; };
  var n = accept;
  while (n && ![].some.call(n.querySelectorAll('button'), isReject)) n = n.parentElement;
  var btn = n && [].find.call(n.querySelectorAll('button'), isReject);
  if (btn) btn.click();
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
			cef->executeJavaScript(kRejectScript);
	}
}

void onFrontendEvent(enum obs_frontend_event event, void *)
{
	if (event != OBS_FRONTEND_EVENT_FINISHED_LOADING || timer)
		return;
	timer = new QTimer(static_cast<QWidget *>(obs_frontend_get_main_window()));
	QObject::connect(timer, &QTimer::timeout, sweep);
	timer->start(2000);
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
