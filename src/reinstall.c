#include "ppkg.h"

int ppkg_reinstall(const char * packageName, const PPKGTargetPlatform * targetPlatform, const PPKGInstallOptions * installOptions) {
    PPKGInstallOptions installOptions2;

    installOptions2.force = true;
    installOptions2.dryrun =    installOptions->dryrun;
    installOptions2.buildType = installOptions->buildType;
    installOptions2.linkType  = installOptions->linkType;
    installOptions2.enableBear  = installOptions->enableBear;
    installOptions2.enableCcache  = installOptions->enableCcache;
    installOptions2.logLevel  = installOptions->logLevel;
    installOptions2.keepSessionDIR  = installOptions->keepSessionDIR;
    installOptions2.parallelJobsCount  = installOptions->parallelJobsCount;
    installOptions2.exportCompileCommandsJson  = installOptions->exportCompileCommandsJson;
    installOptions2.verbose_cc  = installOptions->verbose_cc;
    installOptions2.verbose_ld  = installOptions->verbose_ld;
    installOptions2.verbose_env  = installOptions->verbose_env;
    installOptions2.verbose_net  = installOptions->verbose_net;
    installOptions2.xtrace  = installOptions->xtrace;

    return ppkg_install(packageName, targetPlatform, &installOptions2);
}
