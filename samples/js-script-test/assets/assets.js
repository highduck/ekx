/**
 *
 * @param builder {AssetBuilder}
 */
function on_populate(builder) {
    for (const file of builder.glob("scripts/*.js")) {
        builder.copy({filepath: file});
    }
}

module.exports = {
    on_populate
};