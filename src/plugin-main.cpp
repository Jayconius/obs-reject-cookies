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
#include <QPointer>
#include <QSet>
#include <QTimer>
#include <QWidget>

#include "browser-panel.hpp"

OBS_DECLARE_MODULE()

namespace {

// Installed once per page. On twitch.tv it watches the DOM and clicks the banner's reject
// button as soon as it appears. The button is found by position, not label, so it works in
// any Twitch language: it's the one button next to Accept that isn't Accept or Customize.
const char *kInstallScript = R"JS(
(function () {
  if (window.__obsRejectCookies) return;
  if (!/(^|\.)twitch\.tv$/.test(location.hostname)) return;
  window.__obsRejectCookies = true;

  function tryReject() {
    var accept = document.querySelector('[data-a-target="consent-banner-accept"]');
    if (!accept) return;
    var others = [];
    for (var n = accept.parentElement; n && n !== document.body; n = n.parentElement) {
      others = [].filter.call(n.querySelectorAll('button'), function (b) {
        var t = b.getAttribute('data-a-target');
        return t !== 'consent-banner-accept' && t !== 'consent-banner-manage-preferences';
      });
      if (others.length) break;
    }
    if (others.length === 1) others[0].click();
  }

  tryReject();
  new MutationObserver(tryReject).observe(document.documentElement, { childList: true, subtree: true });
})();
)JS";

QTimer *discoveryTimer = nullptr;
QSet<QWidget *> seen;

// qobject_cast can't be used across the obs-browser DLL boundary, so match by class name instead.
QCefWidget *asCefWidget(QWidget *w)
{
	for (const QMetaObject *m = w->metaObject(); m; m = m->superClass()) {
		if (qstrcmp(m->className(), "QCefWidget") == 0)
			return static_cast<QCefWidget *>(w);
	}
	return nullptr;
}

// A new page may still be loading when its URL changes, so try again a little later too.
// The script's guard makes repeat installs a no-op.
void install(QCefWidget *cef)
{
	QPointer<QCefWidget> guard(cef);
	for (int delay : {0, 1000, 4000}) {
		QTimer::singleShot(delay, cef, [guard] {
			if (guard)
				guard->executeJavaScript(kInstallScript);
		});
	}
}

// Re-installs the script when a dock navigates or reloads. Owned by the dock it watches.
class Reinstaller : public QObject {
	Q_OBJECT

public:
	explicit Reinstaller(QCefWidget *cef) : QObject(cef), cef(cef) {}

public slots:
	void onUrlChanged() { install(cef); }

private:
	QCefWidget *cef;
};

// Picks up docks as they're created; each one is only set up once.
void discover()
{
	for (QWidget *w : QApplication::allWidgets()) {
		if (seen.contains(w))
			continue;
		QCefWidget *cef = asCefWidget(w);
		if (!cef)
			continue;
		seen.insert(w);
		QObject::connect(w, &QObject::destroyed, [w] { seen.remove(w); });
		// String-based connect resolves the signal through obs-browser's own meta-object.
		QObject::connect(w, SIGNAL(urlChanged(QString)), new Reinstaller(cef), SLOT(onUrlChanged()));
		install(cef);
	}
}

void onFrontendEvent(enum obs_frontend_event event, void *)
{
	if (event != OBS_FRONTEND_EVENT_FINISHED_LOADING || discoveryTimer)
		return;
	discoveryTimer = new QTimer(static_cast<QWidget *>(obs_frontend_get_main_window()));
	QObject::connect(discoveryTimer, &QTimer::timeout, discover);
	discoveryTimer->start(1000);
	discover();
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
	if (discoveryTimer)
		discoveryTimer->stop();
	obs_log(LOG_INFO, "plugin unloaded");
}

#include "plugin-main.moc"
