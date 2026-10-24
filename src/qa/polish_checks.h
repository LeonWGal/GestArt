#pragma once
#include <QVariantMap>
class QQuickWindow;
class Controller;
void installPolishChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);
void installGlassChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);
void installInteractionChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);
void installLobeChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);

void installPreviewChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);

void installControlChecks(QQuickWindow* window,Controller* controller,QVariantMap* metrics);
