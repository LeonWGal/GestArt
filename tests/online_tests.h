#pragma once
#include <QObject>
class OnlineTests : public QObject {
    Q_OBJECT
private slots:
    void entirePackUsesAllReferencesAndPersists();
    void cachedProkoCatalogKeepsTheAnonymousSession();
    void partialPacksCanResumeAfterRestart();
    void prokoEntirePackReadsEveryPage();
    void randomSourcesRejectEntirePack();
    void savedSetsAreDiscoveredWithoutQueueHistory();
    void aNewDownloadDoesNotRestartOldJobs();
    void downloadsRunInParallelAndRetry();
    void practiceStartsOnlyForItsJob();
    void cancelAbortsAllImagesAndKeepsCompleted();
    void stalledRequestsHaveAnAbsoluteDeadline();
    void catalogIsReusedUntilRefresh();
    void filtersUseProviderValues();
    void queueKeepsDifferentFilters();
    void batchQueueLimitsAreAtomic();
    void batchQueueDeduplicatesAndRemoves();
    void downloadDestinationPersists();
    void cancellingBeforeFirstRequestDoesNotResume();
    void supportedSites();
    void linksStayOnTrustedHttpsHosts();
    void quickposesRejectsInvalidAndDuplicateEntries();
    void prokoUsesFullDownloadInsteadOfCover();
    void poseLibraryUsesFullImage();
    void sketchDailyKeepsCreditAndRejectsTraversal();
    void malformedRepliesAreEmpty();
    void invalidActionsDoNotStartRequests();
};
