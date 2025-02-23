import { use } from "react";
import { Game } from "./game";

export default function Page({searchParams}: { searchParams: Promise<object> }) {
	const params = use(searchParams);
	return <Game params={params} />;
}