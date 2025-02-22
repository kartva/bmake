export type ServerResponse<T> = {
  status:"error",
  error: "notFound"|"unauthorized"|"badRequest"|"loading"
    |"rateLimited"|"other"|"sessionExpire"|"banned",
  message: string|null
} | {status: "ok", result: T}

export function errorName(err: (ServerResponse<unknown>&{status:"error"})["error"]) {
  let name = "Unknown error";
  switch (err) {
    case "badRequest": name = "Bad Request"; break;
    case "loading": name = "Loading"; break;
    case "notFound": name = "Not Found"; break;
    case "other": name = "Other Error"; break;
    case "rateLimited": name = "Rate Limited"; break;
    case "banned": name = "You've been banned!"; break;
    case "sessionExpire": name = "Session expired"; break;
    case "unauthorized": name = "Unauthorized"; break;
  }
  return name;
}
