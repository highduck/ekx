#!/usr/bin/env -S npx ts-node

import {spawnSync} from "child_process";
import {collectPackages, upgradePackages} from "./ws.js";
import {logger} from "../cli/logger.js";

upgradePackages(collectPackages("."));

// finally update super space
logger.info("final step install outer super-repo");

spawnSync("npm i", {
    stdio: 'inherit'
});