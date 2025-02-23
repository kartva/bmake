import { Popover, PopoverContent, PopoverTrigger } from "@heroui/popover";
import { Spinner, SpinnerProps } from "@heroui/spinner";
import { IconChevronDown } from "@tabler/icons-react";
import Image from "next/image";
import { LinkProps } from "next/link";
import React, { AnchorHTMLAttributes, forwardRef, HTMLAttributes, JSX } from "react";
import { ClassNamesConfig, Props as SelectProps } from "react-select";
import { ClassNameValue, twMerge } from "tailwind-merge";
import { AppLink } from "./clientutil";
import { Footer } from "./footer";
import star from "./star.svg";

import names from "./names.json";
export const namePaths = Object.fromEntries(names.map(str=>[str,`/icon/${str}.svg`]));

export const textColor = {
	contrast: "dark:text-white text-black",
	sky: "dark:text-sky-400 text-sky-700",
	green: "dark:text-green-500 text-green-700",
	red: "dark:text-red-500 text-red-700",
	default: "dark:text-zinc-100 text-zinc-800 dark:disabled:text-gray-400 disabled:text-gray-500",
	link: "text-gray-700 dark:text-gray-200 underline-offset-2 transition-all hover:text-black dark:hover:text-gray-50 hover:bg-cyan-800/5 dark:hover:bg-cyan-100/5 cursor-pointer underline decoration-dashed decoration-1",
	blueLink: "dark:text-blue-200 text-sky-800",
	star: "dark:text-amber-400 text-amber-600",
	gray: "dark:text-gray-200 text-gray-700"
};

export const bgColor = {
	default: "dark:bg-zinc-800 bg-zinc-200 dark:disabled:bg-zinc-600",
	md: "dark:bg-zinc-850 bg-zinc-150 dark:disabled:bg-zinc-600",
	hover: "dark:bg-zinc-700 bg-zinc-150",
	secondary: "dark:bg-zinc-900 bg-zinc-150",
	green: "dark:enabled:bg-green-600 enabled:bg-green-400",
	sky: "dark:enabled:bg-sky-600 enabled:bg-sky-300",
	red: "dark:enabled:bg-red-800 enabled:bg-red-300",
	rose: "dark:enabled:bg-rose-900 enabled:bg-rose-300",
	highlight: "dark:bg-amber-600 bg-amber-200",
	restriction: "dark:bg-amber-900 bg-amber-100",
	divider: "dark:bg-zinc-500 bg-zinc-400",
	contrast: "dark:bg-white bg-black"
}

export const borderColor = {
	default: "border-zinc-300 dark:border-zinc-600 disabled:bg-zinc-300 aria-expanded:border-blue-500 data-[selected=true]:border-blue-500 outline-none",
	red: "border-red-400 dark:border-red-600",
	defaultInteractive: "focus:outline-none border-zinc-300 hover:border-zinc-400 dark:border-zinc-600 dark:hover:border-zinc-500 disabled:bg-zinc-300 aria-expanded:border-blue-500 focus:border-blue-500 active:border-blue-500 dark:focus:border-blue-500 dark:active:border-blue-500 data-[selected=true]:border-blue-500 outline-none",
};

export const containerDefault = `${textColor.default} ${bgColor.default} ${borderColor.default} rounded-md border-1`;
export const interactiveContainerDefault = `${textColor.default} ${bgColor.default} ${borderColor.defaultInteractive} border-1`;

export const Anchor: React.FC<(AnchorHTMLAttributes<HTMLAnchorElement>)&Partial<LinkProps>> = ({className,href,...props}) => {
	const classN = twMerge(
		`inline-flex flex-row align-baseline items-baseline gap-1 cursor-pointer`, textColor.link, className
	);

	//special protocols which nextjs link can't handle...
	if (href?.startsWith("mailto:"))
		return <a className={classN} href={href} {...props} >{props.children}</a>
	else if (href!=undefined)
		return <AppLink href={href} rel="noopener noreferrer" className={classN} {...props} >
			{props.children}
		</AppLink>;
	else return <a className={classN} href="#" {...props} onClick={(ev) => {
		ev.preventDefault();
		props.onClick?.(ev);
	}} >{props.children}</a>
}

export type InputProps = {icon?: React.ReactNode}&React.InputHTMLAttributes<HTMLInputElement>;
export const Input = React.forwardRef<HTMLInputElement, InputProps>(
	function Input({className, icon, ...props}, ref) {
		return <div className={twMerge("relative", className)} >
			<input ref={ref} type="text" className={`w-full p-2 border-2 transition duration-300 rounded-lg ${icon ? "pl-11" : ""} ${interactiveContainerDefault}`} {...props} />
			{icon && <div className="absolute left-0 my-auto pl-3 top-0 bottom-0 flex flex-row items-center" >
				{icon}
			</div>}
		</div>;
	}
);

export const Textarea = forwardRef<HTMLTextAreaElement, JSX.IntrinsicElements["textarea"]>(function Textarea(
	{className, children, ...props}: JSX.IntrinsicElements["textarea"], ref
) {
	return <textarea className={twMerge("w-full p-2 border-2 transition duration-300 rounded-lg resize-y max-h-60 min-h-24", interactiveContainerDefault, className)}
		rows={6} {...props} ref={ref} tabIndex={100} >
		{children}
	</textarea>;
});

export const Button = ({className, disabled, icon, ...props}: HTMLAttributes<HTMLButtonElement>&{icon?: React.ReactNode, disabled?: boolean}) =>
	<button disabled={disabled} className={twMerge("flex flex-row justify-center gap-1.5 px-4 py-1.5 items-center rounded-xl group", interactiveContainerDefault, icon ? "pl-3" : "", className)} {...props} >
		{icon}
		{props.children}
	</button>;

export const HiddenInput = ({className, ...props}: React.InputHTMLAttributes<HTMLInputElement>) =>
	<input className={twMerge(`bg-transparent border-0 outline-none border-b-2
		focus:outline-none focus:border-blue-500 transition duration-300 px-1 py-px`, borderColor.default, className)}
		{...props} ></input>

export const IconButton = ({className, icon, disabled, ...props}: {icon?: React.ReactNode, disabled?: boolean}&Omit<JSX.IntrinsicElements["button"],"children">) =>
	<button className={twMerge("rounded-full p-2 flex items-center justify-center", interactiveContainerDefault, className)} disabled={disabled} {...props} >
		{icon}
	</button>;

export const LinkButton = ({className, icon, ...props}: React.AnchorHTMLAttributes<HTMLAnchorElement>&{icon?: React.ReactNode}) =>
	<a className={twMerge('flex flex-row gap-2 px-3 py-1.5 items-center rounded-xl text-sm',interactiveContainerDefault,className)} rel="noopener noreferrer" {...props} >
		{icon &&
			<span className="inline-block h-4 w-auto" >{icon}</span> }
		{props.children}
	</a>

export const ThemeSpinner = (props: SpinnerProps) =>
	<Spinner classNames={{
		circle1: "dark:border-b-white border-b-blue-600",
		circle2: "dark:border-b-white border-b-blue-600"
	}} {...props} />;

export const Loading = (props: SpinnerProps) => <div className="h-full w-full flex item-center justify-center py-16 px-20" >
	<ThemeSpinner size="lg" {...props} />
</div>

export const chipColors = {
	red: "dark:bg-red-600 dark:border-red-400 bg-red-400 border-red-200",
	green: "dark:bg-green-600 dark:border-green-400 bg-green-400 border-green-200",
	blue: "dark:border-cyan-400 dark:bg-sky-600 border-cyan-200 bg-sky-400",
	gray: "dark:border-gray-300 dark:bg-gray-600 border-gray-100 bg-gray-300",
	purple: "dark:bg-purple-600 dark:border-purple-300 bg-purple-400 border-purple-300",
	teal: "dark:bg-[#64919b] dark:border-[#67cce0] bg-[#aedbe8] border-[#95e6fc]",
};

export const chipColorKeys = Object.keys(chipColors) as (keyof typeof chipColors)[];

export const Chip = ({className, color, ...props}: HTMLAttributes<HTMLSpanElement>&{color?: keyof typeof chipColors}) =>
	<span className={twMerge("inline-block text-xs px-2 py-1 rounded-lg border-solid border whitespace-nowrap", chipColors[color ?? "gray"], className)}
		{...props} >{props.children}</span>

const Star = ()=><svg xmlns="http://www.w3.org/2000/svg" xmlSpace="preserve" style={{
  fillRule: "evenodd",
  clipRule: "evenodd",
  strokeLinejoin: "round",
  strokeMiterlimit: 2
}} className="w-14" viewBox="0 0 150 150" ><path d="M-24.188 8.556h313.889v121.588H-24.188z" style={{
    fill: "none"
  }} transform="matrix(.47788 0 0 1.23367 11.559 -10.555)" /><path d="M31.155 65.897c-.161-.743.364-1.614.836-2.124.472-.509 1.276-1.08 1.996-.933 1.786.365 7.31 3.631 8.721 3.122 1.411-.51-.351-4.806-.257-6.179.05-.738.343-1.582.82-2.06.478-.477 1.357-.804 2.044-.804.686 0 1.593.327 2.076.804.482.478.778 1.322.82 2.06.086 1.505-2.072 3.26-.306 6.971.756 1.59 7.971-2.385 9.961-2.691.71-.109 1.518.365 1.979.853.461.488 1.024 1.374.789 2.076-.271.807-1.269 2.334-2.414 2.767-1.727.655-7.3.236-7.949 1.159-.649.922 2.674 2.973 4.055 4.377 1.369 1.391 3.797 2.832 4.233 4.046.409 1.137-.803 2.441-1.614 3.243s-2.167 1.567-3.253 1.571c-1.086.004-3.253-.343-3.263-1.547-.022-2.572-1.875-9.162-4.825-8.247-1.819.565-4.724 8.163-6.561 9.262-1.487.89-3.57-1.877-4.461-2.664-.56-.495-.901-1.313-.885-2.06.018-.866.128-2.465.994-3.14 1.451-1.132 7.997-2.426 7.713-3.651-.283-1.226-5.016-4.614-9.415-3.7-1.015.21-1.668-1.738-1.834-2.511Z" style={{
    fillRule: "nonzero"
  }} transform="matrix(3.09015 3.05585 -3.72352 2.87586 201.187 -271.363)" /></svg>;

export const LogoText = ({className, ...props}: HTMLAttributes<HTMLHRElement>) => 
	<AppLink href="/" >
		<h1 className={twMerge("text-4xl md:text-6xl mr-2 my-auto select-none cursor-pointer font-display font-black fill-white flex flex-row gap-1 items-center", className)} {...props} >
			<Star/> fish
		</h1>
	</AppLink>;

export const StatusPage = ({children, title}: {children: React.ReactNode, title: string}) =>
	<>
		<div className="flex h-dvh flex-col justify-between items-center py-20 px-5" >
			<AppLink href="/" ><LogoText/></AppLink>
			<div className="flex flex-col lg:w-96" >
				<h1 className="font-display font-extrabold text-4xl mb-4" >{title}</h1>
				<div className="flex flex-col gap-4">
					{children}
				</div>
			</div>

			<Footer/>
		</div>
	</>;

export function ButtonPopover({children, className, title, desc}: {children: React.ReactNode, className?: ClassNameValue, title: string, desc?: string}) {
	return <Popover placement="bottom" showArrow triggerScaleOnOpen={false} >
		<PopoverTrigger>
			<div className="flex-1 md:flex-none" >
				<Button className={twMerge("w-full h-full pr-3 justify-between", className)} >
					<div className="flex flex-col items-start justify-center" >
						<span>{title}</span>
						<Text v="dim" className="whitespace-nowrap" >{desc}</Text>
					</div>
					<IconChevronDown />
				</Button>
			</div>
		</PopoverTrigger>
		<PopoverContent className={`${bgColor.secondary} ${borderColor.default} p-5 pt-3`} >
			{children}
		</PopoverContent>
	</Popover>;
}

const selectStyle: ClassNamesConfig<unknown,boolean> = {
	control: (state) => `flex flex-row gap-4 px-3 py-1.5 dark:bg-zinc-800 bg-zinc-200 items-center border-2 text-zinc-800 dark:text-white rounded-lg hover:cursor-pointer ${state.menuIsOpen ? "dark:border-blue-500 border-blue-400" : "dark:border-zinc-600 border-zinc-300"}`,
	menuList: () => "border-2 border-zinc-300 dark:border-zinc-700 rounded-lg bg-zinc-100 dark:bg-black mt-1 flex flex-col items-stretch",
	option: ({ isDisabled, isFocused }) => {
		return `${isFocused ? "dark:bg-zinc-800 bg-zinc-300" : ""} hover:bg-zinc-300 dark:hover:bg-zinc-800 p-2 border-t first:border-none dark:border-zinc-700 border-zinc-300 hover:cursor-pointer ${isDisabled ? "dark:text-gray-500 text-gray-400" : ""}`;
	},
	menu: () => "dark:text-white text-zinc-800 absolute w-full",
	multiValue: () => "dark:bg-zinc-700 bg-zinc-300 dark:text-white text-zinc-800 px-2 py-0.5 rounded-md",
	multiValueLabel: () => "dark:text-white text-zinc-800 dark:hover:bg-zinc-700 hover:bg-zinc-300",
	valueContainer: () => "flex flex-row gap-1 overflow-x-auto",
	multiValueRemove: () => "dark:text-white text-zinc-800 dark:hover:bg-zinc-700 hover:bg-zinc-300 dark:hover:text-white hover:text-zinc-800 ml-1",
	indicatorSeparator: () => "mx-1 h-full dark:bg-zinc-600 bg-zinc-300",
	input: () => "dark:text-white text-zinc-800",
	noOptionsMessage: () => "py-2 text-zinc-800 dark:text-white",
	indicatorsContainer: () => "dark:text-white text-zinc-800",
}

const infiniteZIndexHack = (base: object) => ({...base, zIndex: 1000});
export const selectProps = <T,Multi extends boolean>(): SelectProps<T,Multi> => ({
	unstyled: true, classNames: selectStyle as ClassNamesConfig<T,Multi>,
	styles: {
		menuPortal: infiniteZIndexHack, menu: infiniteZIndexHack
	}
});

export const firstLast = (s: string) => {
	const x = s.split(/\s+/);
	return x.length>=2 ? `${x[0]} ${x[x.length-1]}` : x[0];
};

export function shitHash(s: string) {
	const mod = 17;
	let v=3, o=0;
	for (let i=0; i<s.length; i++) {
		o+=s.charCodeAt(i)*v;
		if (o>=mod) o-=mod;
		v=3*v %mod;
	}
	return o;
}

export function capitalize(s: string) {
	const noCap = ["of", "a", "an", "the", "in"];
	return s.split(/\s+/g).filter(x=>x.length>0).map((x,i)=>{
		if (i>0 && noCap.includes(x)) return x;
		else return `${x[0].toUpperCase()}${x.slice(1)}`;
	}).join(" ");
}

type TextVariants = "big"|"lg"|"md"|"dim"|"bold"|"normal"|"err"|"sm"|"smbold";
export function Text({className, children, v, ...props}: HTMLAttributes<HTMLParagraphElement>&{v?: TextVariants}) {
	switch (v) {
		case "big": return <h1 {...props} className={twMerge("md:text-3xl text-2xl font-display font-black", textColor.contrast, className)} >{children}</h1>;
		case "bold": return <b {...props} className={twMerge("text-lg font-display font-extrabold", textColor.contrast, className)} >{children}</b>;
		case "smbold": return <b className={twMerge("text-sm font-display font-bold text-gray-700 dark:text-gray-300", className)} {...props} >{children}</b>;
		case "md": return <h3 {...props} className={twMerge("text-xl font-display font-bold", textColor.contrast, className)} >{children}</h3>;
		case "lg": return <h3 {...props} className={twMerge("text-xl font-display font-extrabold", textColor.contrast, className)} >{children}</h3>;
		case "dim": return <span {...props} className={twMerge("text-sm text-gray-500 dark:text-gray-400", className)} >{children}</span>;
		case "sm": return <p {...props} className={twMerge("text-sm text-gray-800 dark:text-gray-200", className)} >{children}</p>;
		case "err": return <span {...props} className={twMerge("text-red-500", className)} >{children}</span>;
		default: return <p {...props} className={className} >{children}</p>;
	}
}

export const Divider = ({className}: {className?: string}) =>
	<div className={twMerge(`rounded w-px mx-1.5 h-4 self-center flex-shrink-0 ${bgColor.divider}`, className)} />;
	
// export const AppSelect = ({...props}: SelectProps) => <Select {...props}
// 		classNames={{
// 			trigger: twMerge("px-4 py-1.5 bg-zinc-900 border text-white rounded-xl border-zinc-900 hover:border-zinc-700 data-[hover=true]:bg-zinc-900 data-[open=true]:border-blue-500", props.classNames?.trigger)
// 		}}
// 	>
// 	{props.children}
// </Select>