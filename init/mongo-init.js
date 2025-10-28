// 使用 root 身份执行（由官方 entrypoint 自动在首次启动时运行）
const appDb = process.env.MONGO_APP_DB || "cpp-test";
const appUser = process.env.MONGO_APP_USER || "appuser";
const appPass = process.env.MONGO_APP_PASS || "appsecret";

print(`Creating app user '${appUser}' on db '${appDb}' ...`);

db = db.getSiblingDB(appDb);
db.createUser({
    user: appUser,
    pwd: appPass,
    roles: [{ role: "readWrite", db: appDb }],
});

// 可选：初始化集合
db.createCollection("cpp");
print("Mongo init done.");