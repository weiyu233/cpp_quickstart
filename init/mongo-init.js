db = db.getSiblingDB('bond_demo');
db.createUser({
    user: "weiyu",
    pwd: "secret123",
    roles: [ { role: "readWrite", db: "bond_demo" } ]
});