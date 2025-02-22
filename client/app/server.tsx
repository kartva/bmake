/* eslint-disable react/no-unknown-property */
//utilities for server...
//you shouldn't use any of these from non-server components

import { StatusPage, Text } from "@/components/util";
import { headers } from "next/headers";
import { notFound } from "next/navigation";
import { ImageResponse } from "next/og";
import { errorName, ServerResponse } from "@/components/types";

class APIError extends Error {
	constructor(public res: ServerResponse<unknown>&{status: "error"}) {
		super(`couldn't fetch: ${res.error} - ${res.message}`);
	}
};

//a much worse api wrapper for server
export async function api<T,>(endpoint: string, data?: unknown): Promise<T> {
	const v = (await headers()).get("X-Forwarded-For");
	const fetchHdr = new Headers();
	if (v!=null)
		fetchHdr.append("X-Forwarded-For", v.slice(v.lastIndexOf(",")+1));

	const res = await (await fetch(`${process.env.SERVER_URL}/${endpoint}`, {
		method: "POST",
		headers: fetchHdr,
		body: data == undefined ? undefined : JSON.stringify(data),
		cache: "no-cache" //its literally right here
	})).json() as ServerResponse<T>;

	if (res.status == "error") {
		if (res.error == "notFound") notFound();
		throw new APIError(res);
	}

	return res.result;
}

export function catchAPIError<T,A extends unknown[]>(f: (...args: A)=>Promise<T>): (...args: A) => Promise<T|ReturnType<typeof StatusPage>> {
	return async (...args) => {
		try {
			return await f(...args);
		} catch (e) {
			if (e instanceof APIError) return <StatusPage title="An error occurred" >
				<Text v="md" >{errorName(e.res.error)}</Text>
				<Text>{e.res.message ?? "We encountered an error while trying to reach the API"}</Text>
			</StatusPage>
			else throw e;
		}
	};
}
